
class Consumable {
  public:
    int cost;
    bool active;

    Consumable(int cost){
      this->cost = cost;
      this->active = false;
    }

    bool purchase(int &coins){
      if(coins >= cost){
        coins -= this->cost;
        this->active = true;
        return true;
      }
      return false;
    }
};