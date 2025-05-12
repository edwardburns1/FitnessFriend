class StatusBar{
  private:
    int start_x;
    int end_x;
    int curr_end_x;
    uint32_t color;
    int pos_y = 10;
    int width = 6;

  public:
    StatusBar(int start_x, int end_x, uint32_t color){
      this->start_x = start_x;
      this->end_x = end_x;
      this->color = color;
      this->curr_end_x = end_x;
    }

    void setWidth(int width){
      this->width = width;
    }
    void updateState(int val){
      if(val > 100){
        curr_end_x = end_x;
        return;
      }
      curr_end_x = (((end_x - start_x) * val) / 100) + start_x;
    }
    int getStartPos(){
      return start_x;
    }
    int getEndPos(){
      return end_x;
    }
    int getPosY(){
      return pos_y;
    }
    int getWidth(){
      return width;
    }
    int getDisplayPos(){
      return curr_end_x;
    }
    uint32_t getColor(){
      return color;
    }

};
