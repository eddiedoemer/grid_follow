#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/UInt32.h>
#include <std_msgs/UInt8.h>


// Includes for dynamic reconfigure
#include <dynamic_reconfigure/server.h>
#include <grid_follow/GridFollowConfig.h>

#include <nav_msgs/OccupancyGrid.h>


class GridFollow
{
public:
    GridFollow();
    void configCallback(grid_follow::GridFollowConfig &config, uint32_t level);
    void gridCallback(const nav_msgs::OccupancyGrid& grid);
    void signCallBack(const std_msgs::UInt8 val);
    bool SignDetected = 0;

private:

    ros::NodeHandle nh_;
    ros::Publisher pub_;
    ros::Subscriber grid_sub_;
    ros::Subscriber sign_visible_sub;

    dynamic_reconfigure::Server<grid_follow::GridFollowConfig> server_;
    grid_follow::GridFollowConfig config_;

    int dist_;
};

GridFollow::GridFollow() : nh_{"~"}
{
    grid_sub_  = nh_.subscribe("/birds_eye/grid", 10, &GridFollow::gridCallback, this);


    //subscribe to sign visible pub
    sign_visible_sub = nh_.subscribe("/stop_sign_detection/stop_sign", 100, &GridFollow::signCallBack, this);

    // Publish on the twist command topic
    pub_ = nh_.advertise<geometry_msgs::Twist>("/prizm/twist_controller/twist_cmd", 10);

    // Dynamic Reconfigure
    server_.setCallback(boost::bind(&GridFollow::configCallback, this, _1, _2));

    // Load defaults
    server_.getConfigDefault(config_);

    dist_ = 1023;
}

void GridFollow::signCallBack(const std_msgs::UInt8 val)
{
    if(val.data == 1){
        SignDetected = true;
    }
    else{
        SignDetected = false;
    }
}

void GridFollow::configCallback(grid_follow::GridFollowConfig &config, uint32_t level)
{
    config_ = config;
}

void GridFollow::gridCallback(const nav_msgs::OccupancyGrid& grid)
{
    int width = grid.info.width;
    int height = grid.info.height;

    int origin_x = grid.info.origin.position.x;
    int origin_y = grid.info.origin.position.y;

    float speed = 0;
    float turn = 0;
    float dir = 0;

    /*
       0,0    y
         +-------+
       x |       |
         |       |
         +-------+

    */

    /***** YOUR CODE HERE *****/

    int count = 0;
    int linecount = 0;
    int lascount = 0;
    int llinecount = 0;
    int llascount = 0;
    int rlinecount = 0;
    int rlascount = 0;
    int rightcount = 0;
    int leftcount = 0;
    int midcount = 0;
    int mlinecount = 0;
    int mlascount = 0;

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            // Convert x,y to array index
            // grid.data is 1D so we use the following to convert a 2D index to 1D
            int index = x + y * width;

            // Non-zero = occupied
            if (grid.data[index] != 0)
            {
                count++;
            }

            if(grid.data[index] == 127) linecount++;
            if(grid.data[index] == 100) lascount++;
        }
    }

    for (int x = 0; x < width/2; x++)
    {
        for (int y = 0; y < height/2; y++)
        {
            // Convert x,y to array index
            // grid.data is 1D so we use the following to convert a 2D index to 1D
            int index = x + y * width;

            // Non-zero = occupied
            if (grid.data[index] != 0)
            {
                leftcount++;
            }

            if(grid.data[index] == 127) llinecount++;
            if(grid.data[index] == 100) llascount++;
        }
    }
    for (int x = 0; x < width/2; x++)
    {
        for (int y = height/2; y < height; y++)
        {
            // Convert x,y to array index
            // grid.data is 1D so we use the following to convert a 2D index to 1D
            int index = x + y * width;

            // Non-zero = occupied
            if (grid.data[index] != 0)
            {
                rightcount++;
            }

            if(grid.data[index] == 127) rlinecount++;
            if(grid.data[index] == 100) rlascount++;
        }
    }
    for (int x = 30; x < width/2+10; x++)
    {
        for (int y = (height/2)-40; y < (height/2)+40; y++)
        {
            // Convert x,y to array index
            // grid.data is 1D so we use the following to convert a 2D index to 1D
            int index = x + y * width;

            // Non-zero = occupied
            if (grid.data[index] != 0)
            {
                midcount++;
            }

            if(grid.data[index] == 127) mlinecount++;
            if(grid.data[index] == 100) mlascount++;
        }
    }

    mlascount = mlascount * 1.2;
    rlascount = rlascount *1.2;
    llascount = llascount*1.2;
    
    //ROS_ERROR_STREAM("Obstacle Weight " << lascount << " Line Weight " << linecount << " Total " << count);
    ROS_ERROR_STREAM("Left: " << leftcount << " Right: " << rightcount);
    ROS_ERROR_STREAM(midcount);

        speed = .5;

        if(rightcount > leftcount){ 
            dir=1; 
        }else dir=-1;

        if(mlascount > 100){
            turn = .25;
            speed = .4;
        }
        if(mlascount > 1300){
            turn = .7;
            speed = 0;
        }

        if (mlascount < 25) speed = .5;

        if (midcount = 0) speed = .75;

        if (mlinecount > 800){

        turn = .25;

        speed = .4;
        }

        if(SignDetected){
            turn = 0;
            speed = 0;
            ROS_ERROR_STREAM("Sign detected");
        }



     




    ROS_ERROR_STREAM("Speed = " << speed << "Turn " << turn*dir << " Dir " << dir);

    geometry_msgs::Twist twist;
    twist.linear.x = speed;
    twist.angular.z = turn * dir;

    pub_.publish(twist);

    

    /**************************/
}



int main(int argc, char** argv)
{
    ros::init(argc, argv, "grid_follow");

    GridFollow sd{};

    ROS_INFO_STREAM("grid_follow running!");
    ros::spin();
    return 0;
}
