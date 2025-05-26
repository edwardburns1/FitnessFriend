#define STEP_CONVERSION_FACTOR 1 // 100 steps is 1 energy
#define EXERCISE_FACTOR 10
#define CATNIP_FACTOR 5
#define YARN_FACTOR 2

class FitnessManager {
  public:
    uint32_t totalSteps = 0;
    uint32_t recentSteps = 0;
    uint32_t exerciseXP = 0;
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

    void incrementSteps(){
      totalSteps += 1;
      recentSteps += 1;
      if(this->workingOut){
        exerciseXP += 1;
      }
    }


    void startWorkout(){
      this->workingOut = true;
    }

    uint8_t stopWorkout(bool catnip, bool yarn){
      this->workingOut = false;

      uint8_t swole_gained;
      if(yarn){
        swole_gained = exerciseXP / YARN_FACTOR;   
      }
      else if (catnip){
        swole_gained = exerciseXP / CATNIP_FACTOR;
      }
      else {
        swole_gained = exerciseXP / EXERCISE_FACTOR;
      }
      return min(swole_gained, uint8_t(100));
    }

};