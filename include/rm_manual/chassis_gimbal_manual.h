//
// Created by qiayuan on 5/22/21.
//

#ifndef RM_MANUAL_CHASSIS_GIMBAL_MANUAL_H_
#define RM_MANUAL_CHASSIS_GIMBAL_MANUAL_H_

#include "rm_manual/common/manual_base.h"
namespace rm_manual {
class ChassisGimbalManual : public ManualBase {
 public:
  explicit ChassisGimbalManual(ros::NodeHandle &nh) : ManualBase(nh) {
    ros::NodeHandle chassis_nh(nh, "chassis");
    chassis_cmd_sender_ = new rm_common::ChassisCommandSender(chassis_nh, data_.referee_.referee_data_);
    ros::NodeHandle vel_nh(nh, "vel");
    vel_cmd_sender_ = new rm_common::Vel2DCommandSender(vel_nh);
    ros::NodeHandle gimbal_nh(nh, "gimbal");
    gimbal_cmd_sender_ = new rm_common::GimbalCommandSender(gimbal_nh, data_.referee_.referee_data_);
    ui_chassis_ = new UiChassis(&data_.referee_);
    ui_gimbal_ = new UiGimbal(&data_.referee_);
    ui_capacitor_ = new UiCapacitor(&data_.referee_);
    ui_warning_ = new UiWarning(&data_.referee_);
    ui_armor0_ = new UiArmor(&data_.referee_, 0);
    ui_armor1_ = new UiArmor(&data_.referee_, 1);
    ui_armor2_ = new UiArmor(&data_.referee_, 2);
    ui_armor3_ = new UiArmor(&data_.referee_, 3);
  }
 protected:
  void sendCommand(const ros::Time &time) override {
    chassis_cmd_sender_->sendCommand(time);
    vel_cmd_sender_->sendCommand(time);
    gimbal_cmd_sender_->sendCommand(time);
  }
  void updateRc() override {
    ManualBase::updateRc();
    if (std::abs(data_.dbus_data_.wheel) > 0.01) {
      chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::GYRO);
    } else
      chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::FOLLOW);
    vel_cmd_sender_->setAngularZVel(data_.dbus_data_.wheel);
    vel_cmd_sender_->setLinearXVel(data_.dbus_data_.ch_r_y);
    vel_cmd_sender_->setLinearYVel(-data_.dbus_data_.ch_r_x);
    gimbal_cmd_sender_->setRate(-data_.dbus_data_.ch_l_x, -data_.dbus_data_.ch_l_y);
  }
  void updatePc() override {
    ManualBase::updatePc();
    gimbal_cmd_sender_->setRate(-data_.dbus_data_.m_x, data_.dbus_data_.m_y);
  }
  void rightSwitchDown(ros::Duration duration) override {
    ManualBase::rightSwitchDown(duration);
    chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::FOLLOW);
    gimbal_cmd_sender_->setMode(rm_msgs::GimbalCmd::RATE);
  }
  void rightSwitchMid(ros::Duration duration) override {
    ManualBase::rightSwitchMid(duration);
    chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::FOLLOW);
    gimbal_cmd_sender_->setMode(rm_msgs::GimbalCmd::RATE);
  }
  void rightSwitchUp(ros::Duration duration) override {
    ManualBase::rightSwitchUp(duration);
    chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::FOLLOW);
    gimbal_cmd_sender_->setMode(rm_msgs::GimbalCmd::RATE);
    ui_chassis_->setOperateType(UPDATE);
    ui_gimbal_->setOperateType(UPDATE);
  }
  void leftSwitchDown(ros::Duration duration) override {
    ManualBase::leftSwitchDown(duration);
    gimbal_cmd_sender_->setMode(rm_msgs::GimbalCmd::RATE);
  }
  void wPress(ros::Duration /*duration*/) override { vel_cmd_sender_->setLinearXVel(1.); }
  void wRelease(ros::Duration /*duration*/) override { vel_cmd_sender_->setLinearXVel(0.); }
  void aPress(ros::Duration /*duration*/v) override { vel_cmd_sender_->setLinearYVel(1.); }
  void aRelease(ros::Duration /*duration*/) override { vel_cmd_sender_->setLinearYVel(0.); }
  void sPress(ros::Duration /*duration*/) override { vel_cmd_sender_->setLinearXVel(-1.); }
  void sRelease(ros::Duration /*duration*/) override { vel_cmd_sender_->setLinearXVel(0.); }
  void dPress(ros::Duration /*duration*/) override { vel_cmd_sender_->setLinearYVel(-1.); }
  void dRelease(ros::Duration /*duration*/) override { vel_cmd_sender_->setLinearYVel(0.); }
  void xPress(ros::Duration /*duration*/) override {
    ui_chassis_->setOperateType(ADD);
    ui_gimbal_->setOperateType(ADD);
    ui_warning_->setOperateType(ADD);
    ui_capacitor_->setOperateType(ADD);
  }
  void xRelease(ros::Duration /*duration*/) override {
    ui_chassis_->setOperateType(UPDATE);
    ui_gimbal_->setOperateType(UPDATE);
    ui_capacitor_->setOperateType(UPDATE);
  }
  void gPress(ros::Duration /*duration*/) override {
    if (chassis_cmd_sender_->getMsg()->mode == rm_msgs::ChassisCmd::GYRO) {
      chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::FOLLOW);
      vel_cmd_sender_->setAngularZVel(0.);
    } else {
      chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::GYRO);
      vel_cmd_sender_->setAngularZVel(1.);
    }
  }
  void ePress(ros::Duration /*duration*/) override {
    if (chassis_cmd_sender_->getMsg()->mode == rm_msgs::ChassisCmd::TWIST)
      chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::FOLLOW);
    else
      chassis_cmd_sender_->setMode(rm_msgs::ChassisCmd::TWIST);
  }
  void drawUi() override {
    ros::Time time = ros::Time::now();
    ui_chassis_->display(time, chassis_cmd_sender_->getMsg()->mode, chassis_cmd_sender_->getBurstMode());
    ui_gimbal_->display(time, gimbal_cmd_sender_->getMsg()->mode);
    ui_warning_->display(time, chassis_cmd_sender_->getMsg()->mode);
    ui_capacitor_->display(time);
    ui_armor0_->display(time);
    ui_armor1_->display(time);
    ui_armor2_->display(time);
    ui_armor3_->display(time);
  }
  rm_common::ChassisCommandSender *chassis_cmd_sender_{};
  rm_common::Vel2DCommandSender *vel_cmd_sender_;
  rm_common::GimbalCommandSender *gimbal_cmd_sender_{};
  UiChassis *ui_chassis_{};
  UiGimbal *ui_gimbal_{};
  UiWarning *ui_warning_{};
  UiCapacitor *ui_capacitor_{};
  UiArmor *ui_armor0_{};
  UiArmor *ui_armor1_{};
  UiArmor *ui_armor2_{};
  UiArmor *ui_armor3_{};
};
}

#endif //RM_MANUAL_CHASSIS_GIMBAL_MANUAL_H_
