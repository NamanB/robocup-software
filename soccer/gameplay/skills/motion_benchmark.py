import single_robot_composite_behavior
import behavior
from enum import Enum
import main
import constants
import robocup
import skills.move
import time



## Motion Benchmark V0.0.0.0
class MotionBenchmark(single_robot_composite_behavior.SingleRobotCompositeBehavior):

    class State(Enum):
        setup = 1
        noise = 2
        move1 = 3
        BasicMotion = 4
        
    #Latency Measurement
    noiseStartTime = 0.0
    noiseStartPos = None

    noiseMeasured = False
    noiseMaxX = 0.0
    noiseMinX = 0.0
    noiseMaxY = 0.0
    noiseMinY = 0.0

    
    NoiseTest = 0.0
    NoiseTestDone = False
    noiseStartTime = 0.0

    noiseResult = 0.0

    moveStartTime = 0.0
    moveEndTime = 0.0

    #Basic Motion Variables
    

    def __init__(self):
        super().__init__(continuous=False) 


        #Register States
        self.add_state(MotionBenchmark.State.setup,
                       behavior.Behavior.State.running)

        self.add_state(MotionBenchmark.State.noise,
                       behavior.Behavior.State.running)

        self.add_state(MotionBenchmark.State.move1,
                       behavior.Behavior.State.running)
       
        #Add transitions
        self.add_transition(behavior.Behavior.State.start,
                            MotionBenchmark.State.setup, lambda: True,
                            'immediately')

        self.add_transition(MotionBenchmark.State.setup,
                            MotionBenchmark.State.noise,
                            lambda: self.all_subbehaviors_completed(), 'In Position')

        self.add_transition(MotionBenchmark.State.noise,
                            MotionBenchmark.State.move1,
                            lambda: self.noiseMeasured,
                            'The noise has been measured')

        self.add_transition(MotionBenchmark.State.move1,
                            behavior.Behavior.State.completed,
                            lambda: self.all_subbehaviors_completed(),
                            'Noise Test Completed')

    def on_enter_setup(self):
        move_point = robocup.Point(0, constants.Field.Width / 4)

        self.add_subbehavior(skills.move.Move(move_point), 'move') 

    def on_exit_setup(self):
        self.remove_all_subbehaviors()

    def on_enter_noise(self):
        self.noiseStartTime = time.time()
        self.noiseStartPos = self.robot.pos

    def execute_noise(self):
        if(abs(self.noiseStartTime - time.time()) >=  5):
            self.noiseMeasured = True
        deltaX = self.noiseStartPos.x - self.robot.pos.x 
        deltaY = self.noiseStartPos.y - self.robot.pos.y
        if(deltaX > self.noiseMaxX):
            self.noiseMaxX = deltaX
        if(deltaX < self.noiseMinX):
            self.noiseMinX = deltaX
        if(deltaY < self.noiseMinY):
            self.noiseMinY = deltaY
        if(deltaY > self.noiseMaxY):
            self.noiseMaxY = deltaY

    def brokenNoise(self):
        deltaX = self.noiseStartPos.x - self.robot.pos.x 
        deltaY = self.noiseStartPos.y - self.robot.pos.y
        if(deltaX > self.noiseMaxX):
            return True
        if(deltaX < self.noiseMinX):
            return True
        if(deltaY < self.noiseMinY):
            return True
        if(deltaY > self.noiseMaxY):
            return True
        return False

    def on_enter_move1(self):
        self.moveStartTime = time.time()
        self.add_subbehavior(skills.move.Move(robocup.Point(0, constants.Field.Width / 2)), 'move')


    def execute_move1(self):
        noiseTestDone = self.brokenNoise()
        if(noiseTestDone):
            self.remove_all_subbehaviors()

    def on_exit_move1(self):
        self.moveEndTime = time.time()
        self.remove_all_subbehaviors() 
        self.noiseResult = abs(self.moveStartTime - self.moveEndTime)
        print("------------------LATENCY TEST RESULTS---------------------")
        print("Latency (seconds) = " + str(self.noiseResult))
        print("X noise = " + str((abs(self.noiseMaxX) + abs(self.noiseMinX))))
        print("Y noise = " + str((abs(self.noiseMaxY) + abs(self.noiseMinY))))
        print("-----------------------------------------------------------")

    def role_requirements(self):
        reqs = super().role_requirements()
        return reqs
