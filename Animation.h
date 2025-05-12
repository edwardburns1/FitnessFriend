class AnimationBase {
public:
    virtual const unsigned char* getFrame(int frameIndex) = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual int getNumFrames() const = 0;
    virtual bool checkDone() = 0;
};

template <int Width, int Height, int NumFrames>
class Animation : public AnimationBase {
public:
    Animation (const unsigned char (*frames)[Width * Height])
            : frames(frames) {}

    virtual const unsigned char* getFrame(int frameIndex) override {
        lastFrame = frameIndex;
        return frames[frameIndex];
    }

    virtual int getWidth() const override { return Width; }
    virtual int getHeight() const override { return Height; }
    virtual int getNumFrames() const override { return NumFrames; }
    virtual bool checkDone() override { 
      bool done = !((lastFrame + 1) % NumFrames);
      if(done){
        lastFrame = 0;
      }
      return done;
    }
private:
    const unsigned char (*frames)[Width * Height]; // Pointer to the array of frames
    int lastFrame = 0;
};