## MediaPipe to openFrameworks
#### Notes on how to connect Google's [MediaPipe ML Framework](https://github.com/google/mediapipe) to openFrameworks

MediaPipe is a cross-platform framework for building multimodal applied machine learning pipelines. I want to be able to use it in external applications.

This tutorial walks through how to stream MediaPipe data out over UDP, so any external app and receive and use the data.

I show how to modify the mediapipe example _mediapipe/examples/desktop/hand_tracking_ to add in a new node that recieves hand tracking data as input, broadcasts that data over UDP on port 8080, and then passes the tracking data on to the rest of the graph as output.

> Tested on macOS Mojave (10.14.6) and openFrameworks 0.10.1

---
Beigin by installing MediaPipe on your system using [google's instructions](https://mediapipe.readthedocs.io/en/latest/install.html). 

Then install and setup Google [Protobufs](https://developers.google.com/protocol-buffers) for openFrameworks using my previous [tutorial](https://github.com/madelinegannon/protobuf_tutorial).

---
If you've never used [Bazel](https://bazel.build/) before, the build system and organization of Mediapipe can be _really_ confusing. I try to go through step-by-step below, but you can find more information in the [MediaPipe Docs](https://mediapipe.readthedocs.io/en/latest/index.html).

## 1. Update the Graph Definition to Include our new PassThrough Calculator
_Modify mediapipe/graphs/hand_tracking/hand_tracking_desktop_live.pbtxt_
```
# Add New Node
node {
  calculator: "MyPassThroughCalculator"
  input_stream: "LANDMARKS:hand_landmarks"
  input_stream: "NORM_RECT:hand_rect"
  input_stream: "DETECTIONS:palm_detections"
  output_stream: "LANDMARKS:hand_landmarks_out"
  output_stream: "NORM_RECT:hand_rect_out"
  output_stream: "DETECTIONS:palm_detections_out"
}

# Modify input_stream names of next node

# Subgraph that renders annotations and overlays them on top of the input
# images (see renderer_cpu.pbtxt).
node {
  calculator: "RendererSubgraph"
  input_stream: "IMAGE:input_video"
  input_stream: "LANDMARKS:hand_landmarks_out"
  input_stream: "NORM_RECT:hand_rect_out"
  input_stream: "DETECTIONS:palm_detections_out"
  output_stream: "IMAGE:output_video"
}
```
`NOTE` You can visualize the graph to test that inputs and outpus match up at https://viz.mediapipe.dev/

## 2. Modify `my_pass_through_calculator.cc` 
_Add Detections, Landmarks, and Hand Rectangles to Calculator_

1. In `my_pass_though_calculator.cc` include the protobuf headers for your input streams:
```
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/formats/detection.pb.h"
```
2. Declare the Tag for each input stream. These are defined in `.pbtxt` graph file of your demo — for example: `mdeiapipe/graph/hand_tracking/hand_tracking_desktop_live.pbtxt`
```
constexpr char kLandmarksTag[] = "LANDMARKS";
constexpr char kNormLandmarksTag[] = "NORM_LANDMARKS";
constexpr char kNormRectTag[] = "NORM_RECT";
constexpr char kDetectionsTag[] = "DETECTIONS";
```

## 3. Modifying Calculators BUILD file
_Include new dependencies in mediapipe/calculators/core/BUILD_

```
cc_library(
    name = "my_pass_through_calculator",
    srcs = ["my_pass_through_calculator.cc"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/formats:landmark_cc_proto",
        "//mediapipe/framework/formats:rect_cc_proto",
        "//mediapipe/framework/formats:detection_cc_proto",
    ],
    alwayslink = 1,
)
```

You should be able to build and run the mediapipe hand_tracking_desktop_live example now without any errors:

```bash
bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1     mediapipe/examples/desktop/hand_tracking:hand_tracking_cpu
GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/hand_tracking/hand_tracking_cpu     --calculator_graph_config_file=mediapipe/graphs/hand_tracking/hand_tracking_desktop_live.pbtxt
```


## Sending Multiple Protobuf Messages over UDP
There's no good way to send multiple messages (like a LandmarkList, Rect, and DetectionList) in proto2. Therefore, you have to wrap those messages into a new protobuf.

I called mine `wrapper_hand_tracking.proto` and added it and its dependencies to `mediapipe/framework/formats/BUILD`.

Here's a roadblock I hit:

- MediaPipe uses `protobuf-3.11.4` and is built with Bazel, but I'm using `protobuf-3.6.1` for openFrameworks, and it builds with CMake.
- So if I link the `wrapper_hand_tracking.proto` generated with Bazel to the openFrameworks project, there are compatibility issues with the protobuf static library I've already built and linked.

So the hacky workaround I found was to build `wrapper_hand_tracking.proto` with the MediaPipe build system, and then rebuild it with my system-wide protobuf installation. Here's what I did ... in the top-level `/mediapipe` directory:

1. First build with the .proto with bazel.
    - Most of `wrapper_hand_tracking.proto` file is commented out, just leaving the import calls to link to dependent protos.
2. Second, uncomment the .proto and rebuild with `protoc`.
    - For openFrameworks, I've altread built a separate static protobuf library for openFrameworks using [my previous tutorial](https://github.com/madelinegannon/protobuf_tutorial). I need to generate the `.pb.h` and `.pb.cc` files from `wrapper_hand_tracking.proto` using this library.
    - In `wrapper_hand_tracking.proto`, comment out the import calls at the top of the file, and uncomment the body of the proto (`protoc` doesn't link dependencies, so I just copied all the necessary protobufs into this one file).
    - Go into the `mediapipes\framework\formats` directory and run `protoc --cpp_out=. wrapper_hand_tracking.proto`
    - Move those `wrapper_hand_tracking.pb.h` and `wrapper_hand_tracking.pb.cc` files over to your openFrameworks src folder.
    - Drag and drop these two files into your openFrameworks project (be sure to check "Add to Project"). 
     - There shouldn't be any errors now when you add `#include "wrapper_hand_tracking.pb.h"` to `ofApp.h`
     
## Running the Example
When you run the MediaPipe example _hand_tracking_desktop_live_, it broadcasts any hand landmarks and rectangles on port `localhost:8080`. The openFrameworks example _example-protobuf-udp_ is listening for those protobufs on port 8080.

1. Run _hand_tracking_desktop_live_:

```
cd /mediapipe
GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/hand_tracking/hand_tracking_cpu     --calculator_graph_config_file=mediapipe/graphs/hand_tracking/hand_tracking_desktop_live.pbtxt
```
You should see a video feed of yourself, with hand landmarks and bounding rectangle overlaid.

2. Run _example-protobuf-udp_ (cmd-r in xcode)

You should see the numbered landmarks and bounding rectangle on a white screen. 

Press 'SPACE' to use your hand to swat around some particles.

> Note: The example runs on the cpu, so it's a little slow. But the framerate improves a bit once a hand is detected.