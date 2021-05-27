/*
 * ROS node to retrieve data from SBG Ellipse 2N IMU and export it on txt.
 * Author: Javier Gallego Carracedo
 * Email: javiergcrb@gmail.com
 * Organization: MAD Formula Team
 * Date: 27/05/2021
 */

#include "ros/ros.h"
#include "sbg_driver/SbgGpsPos.h"
#include "sbg_driver/SbgGpsVel.h"
#include "sbg_driver/SbgImuData.h"
#include "sbg_driver/SbgStatus.h"
#include "sbg_driver/SbgUtcTime.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int fd_imu_data, fd_pos_data, fd_vel_data;
struct sigaction act;

/**************************************************************
 * DEFINE MESSAGES CALLBACKS
 **************************************************************/
void sbg_imuCallback(const sbg_driver::SbgImuData::ConstPtr& msg)
{
	dprintf(fd_imu_data,"%d,%f,%f,%f,%f,%f,%f\n", msg->time_stamp,
	msg->accel.x, msg->accel.y, msg->accel.z,
	msg->gyro.x, msg->gyro.y, msg->gyro.z);
}

void sbg_gps_posCallback(const sbg_driver::SbgGpsPos::ConstPtr& msg)
{
	dprintf(fd_pos_data,"%d,%f,%f,%f\n", msg->time_stamp,
	msg->position.x, msg->position.y, msg->position.z);
}

void sbg_gps_velCallback(const sbg_driver::SbgGpsVel::ConstPtr& msg)
{
	dprintf(fd_vel_data,"%d,%f,%f,%f\n", msg->time_stamp,
	msg->vel.x, msg->vel.y, msg->vel.z);
}

/******************************************************
 * Function handling SIGINT to close file descriptors before exiting
 ******************************************************/

void close_logs(int n)
{
	ROS_INFO("Closing...");
	close(fd_imu_data);
	close(fd_pos_data);
	close(fd_vel_data);
	exit(0);
}

/*********************************************************
 * MAIN FUNCTION
 *********************************************************/

int main (int argc, char **argv)
{
	time_t t;
	struct tm *tm;
	char filename[100];
	
	t = time(NULL);
	tm = localtime(&t);

	/*
	 * Log files iniziallization
	 */
	
	strftime(filename, 100, "%Y-%m-%d-%H:%M:%S.imu_data.log", tm);
	fd_imu_data = open(filename,O_CREAT|O_RDWR,S_IRUSR|S_IRGRP|S_IROTH);
	dprintf(fd_imu_data,"timestamp,accel.x,accel.y,accel.z,gyro.x,gyro.y,gyro.x\n");
	
	strftime(filename, 100, "%Y-%m-%d-%H:%M:%S.gps_pos.log", tm);
	fd_pos_data = open(filename,O_CREAT|O_RDWR,S_IRUSR|S_IRGRP|S_IROTH);
	dprintf(fd_pos_data,"timestamp,pos.x,pos.y,pos.z\n");

	strftime(filename, 100, "%Y-%m-%d-%H:%M:%S.gps_vel.log", tm);
	fd_vel_data = open(filename,O_CREAT|O_RDWR,S_IRUSR|S_IRGRP|S_IROTH);
	dprintf(fd_vel_data,"timestamp,vel.x,vel.y,vel.z\n");

	ros::init(argc, argv, "sbg2txt");
	ros::NodeHandle n;

	/*
	 * Close signal handling
	 */

	act.sa_handler = close_logs;
	act.sa_flags = SA_RESTART;
	sigaction(SIGINT, &act, NULL);

	ros::Subscriber sub_imu = n.subscribe("sbg/imu_data", 1000, sbg_imuCallback);
	ros::Subscriber sub_gps_pos = n.subscribe("sbg/gps_pos", 1000, sbg_gps_posCallback);
	ros::Subscriber sub_gps_vel = n.subscribe("sbg/gps_vel", 1000, sbg_gps_velCallback);
	ROS_INFO("Running...");
	ros::spin();
	return 0;
}

