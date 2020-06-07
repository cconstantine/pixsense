// ------------------------------------------------ OpenPose C++ Demo ------------------------------------------------
// This example summarizes all the functionality of the OpenPose library. It can...
    // 1. Read a frames source (images, video, webcam, 3D stereo Flir cameras, etc.).
    // 2. Extract and render body/hand/face/foot keypoint/heatmap/PAF of that image.
    // 3. Save the results on disk.
    // 4. Display the rendered pose.
// If the user wants to learn to use the OpenPose C++ library, we highly recommend to start with the examples in
// `examples/tutorial_api_cpp/`.

// Command-line user interface
#include <openpose/flags.hpp>
// OpenPose dependencies
#include <openpose/headers.hpp>
#include <pixsense/face_finder.hpp>

#include <opencv2/opencv.hpp>

void configureWrapper(op::Wrapper& opWrapper)
{
    try
    {
        // Configuring OpenPose

        // logging_level
        op::checkBool(
            0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.",
            __LINE__, __FUNCTION__, __FILE__);
        op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
        op::Profiler::setDefaultX(FLAGS_profile_speed);

        // Applying user defined configuration - GFlags to program variables
        // producerType
        op::ProducerType producerType;
        op::String producerString;
        std::tie(producerType, producerString) = op::flagsToProducer(
            op::String(FLAGS_image_dir), op::String(FLAGS_video), op::String(FLAGS_ip_camera), FLAGS_camera,
            FLAGS_flir_camera, FLAGS_flir_camera_index);
        // cameraSize
        const auto cameraSize = op::flagsToPoint(op::String(FLAGS_camera_resolution), "-1x-1");
        // outputSize
        const auto outputSize = op::flagsToPoint(op::String(FLAGS_output_resolution), "-1x-1");
        // netInputSize
        const auto netInputSize = op::flagsToPoint(op::String(FLAGS_net_resolution), "-1x368");
        // faceNetInputSize
        const auto faceNetInputSize = op::flagsToPoint(op::String(FLAGS_face_net_resolution), "368x368 (multiples of 16)");
        // handNetInputSize
        const auto handNetInputSize = op::flagsToPoint(op::String(FLAGS_hand_net_resolution), "368x368 (multiples of 16)");
        // poseMode
        const auto poseMode = op::flagsToPoseMode(FLAGS_body);
        // poseModel
        const auto poseModel = op::flagsToPoseModel(op::String(FLAGS_model_pose));
        // JSON saving
        if (!FLAGS_write_keypoint.empty())
            op::opLog(
                "Flag `write_keypoint` is deprecated and will eventually be removed. Please, use `write_json`"
                " instead.", op::Priority::Max);
        // keypointScaleMode
        const auto keypointScaleMode = op::flagsToScaleMode(FLAGS_keypoint_scale);
        // heatmaps to add
        const auto heatMapTypes = op::flagsToHeatMaps(FLAGS_heatmaps_add_parts, FLAGS_heatmaps_add_bkg,
                                                      FLAGS_heatmaps_add_PAFs);
        const auto heatMapScaleMode = op::flagsToHeatMapScaleMode(FLAGS_heatmaps_scale);
        // >1 camera view?
        const auto multipleView = (FLAGS_3d || FLAGS_3d_views > 1 || FLAGS_flir_camera);
        // Face and hand detectors
        const auto faceDetector = op::flagsToDetector(FLAGS_face_detector);
        const auto handDetector = op::flagsToDetector(FLAGS_hand_detector);
        // Enabling Google Logging
        const bool enableGoogleLogging = true;

        // Pose configuration (use WrapperStructPose{} for default and recommended configuration)
        const op::WrapperStructPose wrapperStructPose{
            poseMode, netInputSize, outputSize, keypointScaleMode, FLAGS_num_gpu, FLAGS_num_gpu_start,
            FLAGS_scale_number, (float)FLAGS_scale_gap, op::flagsToRenderMode(FLAGS_render_pose, multipleView),
            poseModel, !FLAGS_disable_blending, (float)FLAGS_alpha_pose, (float)FLAGS_alpha_heatmap,
            FLAGS_part_to_show, op::String("/usr/share/openpose/models/"), heatMapTypes, heatMapScaleMode, FLAGS_part_candidates,
            (float)FLAGS_render_threshold, FLAGS_number_people_max, FLAGS_maximize_positives, FLAGS_fps_max,
            op::String(FLAGS_prototxt_path), op::String(FLAGS_caffemodel_path),
            (float)FLAGS_upsampling_ratio, enableGoogleLogging};
        opWrapper.configure(wrapperStructPose);
        // // Face configuration (use op::WrapperStructFace{} to disable it)
        // const op::WrapperStructFace wrapperStructFace{
        //     FLAGS_face, faceDetector, faceNetInputSize,
        //     op::flagsToRenderMode(FLAGS_face_render, multipleView, FLAGS_render_pose),
        //     (float)FLAGS_face_alpha_pose, (float)FLAGS_face_alpha_heatmap, (float)FLAGS_face_render_threshold};
        // opWrapper.configure(wrapperStructFace);
        // // Hand configuration (use op::WrapperStructHand{} to disable it)
        // const op::WrapperStructHand wrapperStructHand{
        //     FLAGS_hand, handDetector, handNetInputSize, FLAGS_hand_scale_number, (float)FLAGS_hand_scale_range,
        //     op::flagsToRenderMode(FLAGS_hand_render, multipleView, FLAGS_render_pose), (float)FLAGS_hand_alpha_pose,
        //     (float)FLAGS_hand_alpha_heatmap, (float)FLAGS_hand_render_threshold};
        // opWrapper.configure(wrapperStructHand);
        // Extra functionality configuration (use op::WrapperStructExtra{} to disable it)
        const op::WrapperStructExtra wrapperStructExtra{
            FLAGS_3d, FLAGS_3d_min_views, FLAGS_identification, FLAGS_tracking, FLAGS_ik_threads};
        opWrapper.configure(wrapperStructExtra);
        // Producer (use default to disable any input)
        // const op::WrapperStructInput wrapperStructInput{
        //     producerType, producerString, FLAGS_frame_first, FLAGS_frame_step, FLAGS_frame_last,
        //     FLAGS_process_real_time, FLAGS_frame_flip, FLAGS_frame_rotate, FLAGS_frames_repeat,
        //     cameraSize, op::String(FLAGS_camera_parameter_path), FLAGS_frame_undistort, FLAGS_3d_views};
        // opWrapper.configure(wrapperStructInput);
        // Output (comment or use default argument to disable any output)
        // const op::WrapperStructOutput wrapperStructOutput{
        //     FLAGS_cli_verbose, op::String(FLAGS_write_keypoint), op::stringToDataFormat(FLAGS_write_keypoint_format),
        //     op::String(FLAGS_write_json), op::String(FLAGS_write_coco_json), FLAGS_write_coco_json_variants,
        //     FLAGS_write_coco_json_variant, op::String(FLAGS_write_images), op::String(FLAGS_write_images_format),
        //     op::String(FLAGS_write_video), FLAGS_write_video_fps, FLAGS_write_video_with_audio,
        //     op::String(FLAGS_write_heatmaps), op::String(FLAGS_write_heatmaps_format), op::String(FLAGS_write_video_3d),
        //     op::String(FLAGS_write_video_adam), op::String(FLAGS_write_bvh), op::String(FLAGS_udp_host),
        //     op::String(FLAGS_udp_port)};
        // opWrapper.configure(wrapperStructOutput);
        // GUI (comment or use default argument to disable any visual output)
        const op::WrapperStructGui wrapperStructGui{
            op::flagsToDisplayMode(FLAGS_display, FLAGS_3d), !FLAGS_no_gui_verbose, FLAGS_fullscreen};
        opWrapper.configure(wrapperStructGui);
        // Set to single-thread (for sequential processing and/or debugging and/or reducing latency)
        if (FLAGS_disable_multi_thread)
            opWrapper.disableMultiThreading();
    }
    catch (const std::exception& e)
    {
        op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
    }
}



class EyeTracker : public Pixsense::AbstractFaceTracker {
public:
  EyeTracker()
  {
    configureWrapper(opWrapper);
    opWrapper.start();
  }

  Pixsense::TrackedFace detect(const cv::Mat& frame, const cv::Mat& depth_frame)
  {
    if(!opWrapper.isRunning()) {
      opWrapper.stop();
      exit(0);
    }
    Pixsense::TrackedFace tf;
    op::opLog("******************** detect ********************", op::Priority::High);

    if(frame.cols > 0 && frame.rows > 0) {
      std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
      std::chrono::time_point<std::chrono::high_resolution_clock> end_time;

      const op::Matrix imageToProcess = OP_CV2OPCONSTMAT(frame);

        // Create new datum
        auto datumsPtr = std::make_shared<std::vector<std::shared_ptr<op::Datum>>>();
        datumsPtr->emplace_back();
        auto& datumPtr = datumsPtr->at(0);
        datumPtr = std::make_shared<op::Datum>();
        datumPtr->cvInputData = imageToProcess;

      opWrapper.tryEmplace(datumsPtr);

      
      start_time = std::chrono::high_resolution_clock::now();

      std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>> datumProcessed;// = opWrapper.emplaceAndPop(datumsPtr);
      opWrapper.waitAndPop(datumProcessed);

      end_time = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float> time_duration = end_time - start_time;
      fprintf(stderr, "%05.5fms\n", time_duration.count() * 1000);

      // if (datumProcessed != nullptr && !datumProcessed->empty())
      // {
      //   fprintf(stderr, "%d\n", datumProcessed->size());
      //   op::opLog("Body keypoints: " + datumProcessed->at(0)->poseKeypoints.toString(), op::Priority::High);
      // }

    }
    return tf;
  }

  op::Wrapper opWrapper{op::ThreadManagerMode::Asynchronous};
  
};


int openPoseDemo()
{
  Pixsense::RealsenseTracker rt;
  glm::vec3 target;

  // Configure OpenPose
  op::opLog("Configuring OpenPose...", op::Priority::High);
  EyeTracker tracker;

  while(true) {
    rt.tick(tracker, target);
  }

  // Return successful message
  return 0;
}

int main(int argc, char *argv[])
{
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // Running openPoseDemo
    return openPoseDemo();
}
