#define STEP_CONVERSION_FACTOR 25 // 100 steps is 1 energy

class FitnessManager {
  private:
    uint32_t totalSteps;
    uint32_t recentSteps;
    uint32_t exerciseXP;
    bool workingOut = false;

  public:
    uint32_t getStepEnergy(){
      uint32_t gainedEnergy = this->recentSteps / STEP_CONVERSION_FACTOR;
      
      this->recentSteps = this->recentSteps % STEP_CONVERSION_FACTOR;
      Serial.printf("Gained %d energy\n", gainedEnergy);
      return gainedEnergy;
    }

    uint32_t updateSteps(uint32_t steps){
      totalSteps += steps;
      recentSteps += steps;
    }

    uint32_t startWorkout(){
      workingOut = true;
    }
}