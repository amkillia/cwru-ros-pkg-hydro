#define _USE_MATH_DEFINES

#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <std_msgs/Float32.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_FLOAT_LEN 32
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/*float motorMove(long double omega) {
    double a = 1.0;
    double t = 0.0;
    double dt = 1.0;
    double x_des = a*sin(omega * t);
    t+=dt;
    return x_des;
}*/

int main(int argc, char **argv) {
    ros::init(argc, argv, "vel_scheduler"); // name of this node will be "minimal_publisher1"
    ros::NodeHandle nh; // get a ros nodehandle; standard yadda-yadda
    //create a publisher object that can talk to ROS and issue twist messages on named topic;
    // note: this is customized for stdr robot; would need to change the topic to talk to jinx, etc.
    ros::Publisher pub  = nh.advertise<std_msgs::Float32>("joint_angle_command", 1);
    //ros::Subscriber sub = nh.subscribe("joint_angle_command", 1, odomCallback);

    long double omega = 3.14;
    double a = 1.0;
    double t = 0.0;
    double dt = 0.1;
    ros::spinOnce(); //allow callbacks to populate fresh data
    
    printf("This is the client program\n");

    int sockfd, portno, n;
    char *name = "192.168.20.5";
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char floatArray[MAX_FLOAT_LEN];


    ROS_INFO("about to create socket");
    //Create socket for client
    portno = 10002;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ROS_INFO("about to enter first if");
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(name);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    //Name the socket as agreed with server
    memset((char *) &serv_addr,0, sizeof(serv_addr)); //sets bytes in memory
    serv_addr.sin_family = AF_INET; //select internet protocol
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,  //address
         server->h_length);
    serv_addr.sin_port = htons(portno); //select port number

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    ROS_INFO("made it to while loop");
    while (ros::ok()) // do work here in infinite loop (desired for this example), but terminate if detect ROS has faulted (or ctl-C)
    { 
        
        std_msgs::Float32 ros_float;
        double x_des = a*sin(omega * t);
        t+=dt;
        ros_float.data = x_des;
        pub.publish(ros_float);
        

        ROS_INFO("about to read and write");
        //actual like code to read and write
        memset(floatArray, 0, sizeof(floatArray));
        snprintf(floatArray,sizeof(floatArray),"%f",ros_float.data);
        ROS_INFO("floatArray %s", floatArray);
        n = send(sockfd,floatArray, strlen(floatArray), 0);
        if (n < 0)
             error("ERROR writing to socket");

         ROS_INFO("move sent to controller");
        /*n = read(sockfd,floatArray,255);
        if (n < 0)
             error("ERROR reading from socket");
        printf("%s\n",floatArray);*/

        ros::Duration(0.1).sleep();

    }  
    ROS_INFO("completed move distance");
    close(sockfd);
    return 0;


    return 0;
}

