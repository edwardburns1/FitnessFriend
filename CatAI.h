
enum AnimationState {STATE_IDLE1, STATE_IDLE2, STATE_WALK, STATE_SLEEP, STATE_EAT, STATE_LICK, STATE_POOP, STATE_SCRATCHING, STATE_BONGO};

int weights = {10, 10, 20, 5, 5, 3, 0, 5, 0};

void catAI(){
  int results[NUM_ANIMATIONS];
  int weights = {10, 10, 20, 5, 5, 3, 0, 5, 0};
  int max_idx;
  int max_val = 0;
  int seed;
  int val;
  for(int i = 0; i < NUM_ANIMATIONS; i++){
    seed = random(10);
    val = seed * weights[i] 
    if(val > max_val){
      max_val = val;
      max_idx = i;
    }
  }
  STATE = max_idx;
}