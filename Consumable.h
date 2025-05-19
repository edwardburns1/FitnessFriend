
class Consumable {
  public:
    int cost;
    bool active = false;

    Consumable(int cost){
      this->cost = cost;
      this->active = false;
    }

    bool purchase(uint8_t &coins){
      if(coins >= cost){
        coins -= this->cost;
        this->active = true;
        return true;
      }
      return false;
    }
};