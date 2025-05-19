#define STEP_CONVERSION_FACTOR 25 // 100 steps is 1 energy

class FitnessManager {
  public:
    uint32_t totalSteps = 0;
    uint32_t recentSteps = 0;
    uint8_t exerciseXP = 0;
    bool workingOut = false;

    void setSteps(uint32_t totalSteps){
      this->totalSteps = totalSteps;
    }

    uint32_t getStepEnergy(){
      uint32_t gainedEnergy = this->recentSteps / STEP_CONVERSION_FACTOR;
      
      this->recentSteps = this->recentSteps % STEP_CONVERSION_FACTOR;
      Serial.printf("Gained %d energy\n", gainedEnergy);
      return min(gainedEnergy, uint32_t(100));
    }

    uint32_t updateSteps(uint32_t steps){
      totalSteps += steps;
      recentSteps += steps;
    }

    void startWorkout(){
      this->workingOut = true;
    }

    uint8_t stopWorkout(){
      this->workingOut = false;
      return min(exerciseXP, uint8_t(100));
    }

};