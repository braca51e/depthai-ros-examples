#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <iostream>
#include <memory>
#include <string>

#include <depthai_bridge/BridgePublisher.hpp>
#include <depthai_bridge/DisparityConverter.hpp>
#include <depthai_bridge/ImageConverter.hpp>
#include "depthai/depthai.hpp"

#ifndef IS_ROS2

#include "depthai_ros_msgs/SetFocus.h"
#include "depthai_ros_msgs/SetExposure.h"
#include "depthai_ros_msgs/SetWhiteBalance.h"
#include "depthai_ros_msgs/SetPostProcessing.h"

#include "ros/ros.h"

template <typename T>
static void getParamWithWarning(ros::NodeHandle& pnh, const char* key, T val) {
    bool gotParam = pnh.getParam(key, val);
    if(!gotParam) {
        std::stringstream ss;
        ss << val;
        ROS_WARN("Could not find param '%s' on node '%s'. Defaulting to '%s'", key, pnh.getNamespace().c_str(), ss.str().c_str());
    }
}
#define req_type bool
using exp_req_msg = depthai_ros_msgs::SetExposure::Request&;
using exp_rep_msg = depthai_ros_msgs::SetExposure::Response&;
using foc_req_msg = depthai_ros_msgs::SetFocus::Request&;
using foc_rep_msg = depthai_ros_msgs::SetFocus::Response&;
using wb_req_msg = depthai_ros_msgs::SetWhiteBalance::Request&;
using wb_rep_msg = depthai_ros_msgs::SetWhiteBalance::Response&;
using pp_req_msg = depthai_ros_msgs::SetPostProcessing::Request&;
using pp_rep_msg = depthai_ros_msgs::SetPostProcessing::Response&;
using ros_node = ros::NodeHandle&;
#define req_get(x) (request.x)
#define rep_get(x) (response.x)
#define set_parameter(a, b) getParamWithWarning(node, a, b)


#else

template <typename T>
static void setRosParameter(std::shared_ptr<rclcpp::Node> node, const char* key, T& val) {
    node->declare_parameter(key, val);
    node->get_parameter(key, val);
}

#include "depthai_ros_msgs/srv/set_focus.hpp"
#include "depthai_ros_msgs/srv/set_exposure.hpp"
#include "depthai_ros_msgs/srv/set_white_balance.hpp"
#include "depthai_ros_msgs/srv/set_post_processing.hpp"
#define req_type void
using exp_req_msg = const std::shared_ptr<depthai_ros_msgs::srv::SetExposure::Request>;
using exp_rep_msg = std::shared_ptr<depthai_ros_msgs::srv::SetExposure::Response>;
using foc_req_msg = const std::shared_ptr<depthai_ros_msgs::srv::SetFocus::Request>;
using foc_rep_msg = std::shared_ptr<depthai_ros_msgs::srv::SetFocus::Response>;
using wb_req_msg = const std::shared_ptr<depthai_ros_msgs::srv::SetWhiteBalance::Request>;
using wb_rep_msg = std::shared_ptr<depthai_ros_msgs::srv::SetWhiteBalance::Response>;
using pp_req_msg = const std::shared_ptr<depthai_ros_msgs::srv::SetPostProcessing::Request>;
using pp_rep_msg = std::shared_ptr<depthai_ros_msgs::srv::SetPostProcessing::Response>;
using ros_node = std::shared_ptr<rclcpp::Node>;
#define req_get(x) ((*request).x)
#define rep_get(x) ((*response).x)
#define set_parameter(a, b) setRosParameter(node, a, b)
#endif

class DepthPostProcessing {
    using TemporalMode = dai::RawStereoDepthConfig::PostProcessing::TemporalFilter::PersistencyMode;
    using DecimationMode = dai::RawStereoDepthConfig::PostProcessing::DecimationFilter::DecimationMode;

    public:
    DepthPostProcessing();
    DepthPostProcessing(ros_node node);
    void setDevice(std::shared_ptr<dai::Device> device);
    // void setFilters();
    dai::RawStereoDepthConfig getFilters(dai::RawStereoDepthConfig config);
    req_type setPostProcessingRequest(pp_req_msg request, pp_rep_msg response);
    void setFilters(dai::RawStereoDepthConfig config);

    private:
    dai::MedianFilter getMedianFilter();
    DecimationMode getDecimationMode();
    TemporalMode getTemporalMode();
    bool _median_enable = false;
    std::string _median_mode = "MEDIAN_OFF";
    bool _speckle_enable = false;
    int _speckle_range = 50;
    bool _temporal_enable = false;
    std::string _temporal_mode = "PERSISTENCY_OFF";
    float _temporal_alpha = 0.4;
    int _temporal_delta = 0;
    bool _spatial_enable = false;
    int _spatial_radius = 2;
    float _spatial_alpha = 0.5;
    int _spatial_delta = 0;
    int _spatial_iterations = 1;
    bool _threshold_enable = false;
    int _threshold_max = 0;
    int _threshold_min = 0;
    bool _decimation_enable = false;
    std::string _decimation_mode = "NON_ZERO_MEDIAN";
    int _decimation_factor = 1;
    std::shared_ptr<dai::Device> _device;
    dai::RawStereoDepthConfig _config;
};

struct ExposureParameters {
    int compensation = 0;
    int time_us = 8333;
    int sensitivity_iso = 100;
    bool auto_exposure = true;
    std::array<int, 4> region = {0, 0, 0, 0};
    std::string name;
};

struct FocusParameters {
    std::string mode = "AUTO";
    std::array<int, 4> region = {0, 0, 0, 0};
    int lens_position = 0;
};

struct WhiteBalanceParameters {
    std::string mode = "AUTO";
    bool lock = false;
    int color_temperature = 6500;
};

class CameraControl {
    public:
    CameraControl();
    CameraControl(ros_node node);
    void setDevice(std::shared_ptr<dai::Device> device);
    // Exposure
    void setExposure();
    req_type setRgbExposureRequest(exp_req_msg request, exp_rep_msg response);
    req_type setStereoExposureRequest(exp_req_msg request, exp_rep_msg response);
    void setRgbExposure(bool value);
    // Focus
    void setFocus();
    req_type setFocusRequest(foc_req_msg request, foc_rep_msg response);
    // White Balance
    void setWhiteBalance();
    req_type setWhiteBalanceRequest(wb_req_msg request, wb_rep_msg response);

    private:
    void set_ros_parameters(ros_node node);
    // Exposure
    ExposureParameters _rgb, _stereo;
    std::shared_ptr<dai::Device> _device;
    bool _exposure_rgb = false;
    void setExposure(ExposureParameters exposure);
    // Focus
    FocusParameters _focus;
    int _lens_position = 0;
    dai::CameraControl::AutoFocusMode getFocusMode();
    // White balance
    dai::CameraControl::AutoWhiteBalanceMode getWhiteBalanceMode();
    std::string _white_balance_mode = "AUTO";
    int _color_temperature_k = 6000;
};

#endif
