// Command-line user interface
#include <openpose/flags.hpp>
// OpenPose dependencies
#include <pixsense/eye_tracker.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#define GLM_ENABLE_EXPERIMENTAL 1

#include <opencv2/highgui.hpp>
#include <glm/gtx/string_cast.hpp>
namespace Pixsense {

  void draw_rectangle(cv::Mat& image, cv::Rect rect, cv::Scalar color = cv::Scalar(255,255,255)) {
    int x1 = (int)(rect.x);
    int y1 = (int)(rect.y);
    int x2 = (int)((rect.x + rect.width));
    int y2 = (int)((rect.y + rect.height));

    //

    cv::rectangle(image, cv::Point(x1, y1), cv::Point(x2, y2), color, 5, 4);
  }

  Person::Person() { }
  Person::Person(const Person& p) : left_eye(p.left_eye), right_eye(p.right_eye) {  }
  Person::Person(glm::vec2 left_eye, glm::vec2 right_eye) : left_eye(left_eye), right_eye(right_eye) { }

  glm::vec2 Person::midpoint() {
    glm::vec2 sum = left_eye + right_eye;
    return glm::vec2(sum.x / 2.0f, sum.y / 2.0f);
  }

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
          FLAGS_part_to_show, op::String("/openpose/models/"), heatMapTypes, heatMapScaleMode, FLAGS_part_candidates,
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
      // const op::WrapperStructExtra wrapperStructExtra{
      //     FLAGS_3d, FLAGS_3d_min_views, FLAGS_identification, FLAGS_tracking, FLAGS_ik_threads};
      // opWrapper.configure(wrapperStructExtra);
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
    } catch (const std::exception& e) {
        op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
    }
  }

  EyeTracker::EyeTracker() : should_exit(false)
  {
    configureWrapper(opWrapper);
    opWrapper.start();
  }

  bool EyeTracker::detect(const rs2::frameset &unaligned_frames, cv::Rect& detection)
  {
    bool is_detected = false;

    if(!opWrapper.isRunning()) {
      opWrapper.stop();
      should_exit = true;
      return false;
    }
    // rs2::align align(rs2_stream::RS2_STREAM_COLOR);
    // rs2::frameset aligned_frames = align.process(unaligned_frames);
    // rs2::video_frame images = aligned_frames.get_color_frame();
    rs2::depth_frame depths = unaligned_frames.get_depth_frame();
    rs2::video_frame ir_frame = unaligned_frames.get_infrared_frame(1);
    // cv::Mat image_matrix = RealsenseTracker::frame_to_mat(images);
    // cv::Mat depth_matrix = RealsenseTracker::frame_to_mat(depths);
    cv::Mat grays_matrix = AbstractFaceTracker::frame_to_mat(ir_frame);

    cv::Mat frame;
    cvtColor(grays_matrix, frame, cv::COLOR_GRAY2BGR);

    if(frame.cols > 0 && frame.rows > 0) {
      if (previous_frame.cols > 0 && previous_frame.rows > 0) {
        cv::Mat frameDelta;
        
        cv::absdiff(previous_frame, grays_matrix, frameDelta);
        double min, max;
        cv::minMaxLoc(frameDelta, &min, &max);

        if (max < 50) {
          return false;
        }
      }
      grays_matrix.copyTo(previous_frame);

      const op::Matrix imageToProcess = OP_CV2OPCONSTMAT(frame);
      std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>> datumProcessed = opWrapper.emplaceAndPop(imageToProcess);

      if (datumProcessed != nullptr && !datumProcessed->empty() )
      {
        std::shared_ptr<op::Datum> match = datumProcessed->at(0);

        op::Array<float> pose_keypoints = match->poseKeypoints;

        if (pose_keypoints.getSize(0) > 0) {
          std::vector<Person> persons;
          // persons.resize(pose_keypoints.getSize(0));

          for(int i = 0;i < pose_keypoints.getSize(0);i++) {
            if (pose_keypoints[i*75 + 16*3 + 2] > 0.f &&
                pose_keypoints[i*75 + 15*3 + 2] > 0.f) {
              persons.push_back(Person(
                glm::vec2(pose_keypoints[i*75 + 16*3], pose_keypoints[i*75 + 16*3 + 1]),
                glm::vec2(pose_keypoints[i*75 + 15*3], pose_keypoints[i*75 + 15*3 + 1])
              ));
            }
          }
          float distance = INFINITY;
          Person previous_selection = selected_person;

          for(int i = 0;i < persons.size();i++) {
            float dist = glm::distance(previous_selection.midpoint(), persons[i].midpoint());
            if (dist < distance) {
              distance = dist;
              selected_person = persons[i];
            }
          }

          float length = glm::distance(selected_person.left_eye, selected_person.right_eye);

          detection.x = selected_person.midpoint().x - length / 2;
          detection.y = selected_person.midpoint().y - length / 2;
          detection.width = length;
          detection.height = length;
          is_detected = true;
        }
      }

    }

    return is_detected;
  }

}