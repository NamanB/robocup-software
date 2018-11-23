#include "WorldRobot.hpp"
#include <iostream>
#include <cmath>

void WorldRobot::createConfiguration(Configuration* cfg) {
    robot_merger_power = new ConfigDouble(cfg, "VisionFilter/WorldRobot/robot_merger_power", 1.5,)
}

WorldRobot::WorldRobot() : isValid(false) {}

WorldRobot::WorldRobot(int robotID, std::list<KalmanRobot> kalmanRobots)
    : robotId(robotID), isValid(true) {

    Geometry2d::Point posAvg = Geometry2d::Point(0, 0);
    double thetaAvg = 0;
    Geometry2d::Point velAvg = Geometry2d::Point(0, 0);
    double omegaAvg = 0;

    double totalPosWeight = 0;
    double totalVelWeight = 0;

    // Below 1 would invert the ratio of scaling
    // Above 2 would just be super noisy
    if (robot_merger_power < 1 || robot_merger_power > 2) {
        std::cout
             << "CRITICAL ERROR: robot_merger_power must be between 1 and 2"
             << std::endl;
    }

    if (kalmanRobots.size() == 0) {
        std::cout
             << "CRITICAL ERROR: Zero robots are given to the WorldRobot constructor"
             << std::endl;

        pos   = posAvg;
        theta = thetaAvg;
        vel   = velAvg;
        omega = omegaAvg;
        posCov = 0;
        velCov = 0;
        isValid = false;

        return;
    }

    for (KalmanRobot& robot : kalmanRobots) {
        // Get the covariance of everything
        // AKA how well we can predict the next measurement
        Geometry2d::Point posCov = robot.getPosCov();
        Geometry2d::Point velCov = robot.getVelCov();
        double thetaCov          = robot.getThetaCov();
        double omegaCov          = robot.getOmegaCov();

        // Std dev of each state
        // Lower std dev gives better idea of true values
        Geometry2d::Point posStdDev;
        Geometry2d::Point velStdDev;
        double thetaStdDev;
        double omegaStdDev;
        posStdDev.x() = std::sqrt(posCov.x());
        posStdDev.y() = std::sqrt(posCov.y());
        velStdDev.x() = std::sqrt(velCov.x());
        velStdDev.y() = std::sqrt(velCov.y());
        thetaStdDev   = std::sqrt(thetaCov);
        omegaStdDev   = std::sqrt(thetaCov);

        // Inversely proportional to how much the filter has been updated
        double filterUncertantity = 1 / robot.getHealth();

        // How good of pos/vel estimation in total
        // (This is less efficient than just doing the sqrt(x_cov + y_cov),
        //  but it's a little more clear math-wise)
        double posUncertantity = std::sqrt(posStdDev.magsq() + thetaStdDev*thetastdDev);
        double velUncertantity = std::sqrt(posStdDev.magsq() + omegaStdDev*omegaStdDev);

        double filterPosWeight = std::pow(posUncertantity * filterUncertantity,
                                          -*robot_merger_power);

        double filterVelWeight = std::pow(velUncertantity * filterUncertantity,
                                          -*robot_merger_power);

        // TODO: Constrain Theta
        posAvg   += filterPosWeight * robot.getPos();
        thetaAvg += filterPosWeight * robot.getTheta();
        velAvg   += filterVelWeight * robot.getVel();
        omegaAvg += filterVelWeight * robot.getOmega();

        totalPosWeight += filterPosWeight;
        totalVelWeight += filterVelWeight;
    }

    posAvg   /= totalPosWeight;
    thetaAvg /= totalPosWeight;
    velAvg   /= totalVelWeight;
    omegaAvg /= totalVelWeight;

    pos   = posAvg;
    theta = thetaAvg;
    vel   = velAvg;
    omega = omegaAvg;
    posCov = totalPosWeight / kalmanRobots.size();
    velCov = totalVelWeight / kalmanRobots.size();
    robotComponents = kalmanRobots;
}

bool WorldRobot::getIsValid() {
    return isVAlid;
}

int WorldRobot::getRobotID() {
    return robotID;
}

Geometry2d::Point WorldRobot::getPos() {
    return pos;
}

double WorldRobot::getTheta() {
    return theta;
}

Geomtery2d::Point WorldRobot::getVel() {
    return vel;
}

double WorldRobot::getOmega() {
    return omega;
}

double WorldRobot::getPosCov() {
    return posCov;
}

double getVelCov() {
    return velCov;
}

std::list<KalmanRobots> WorldRobot::getRobotComponents() {
    return robotComponents;
}