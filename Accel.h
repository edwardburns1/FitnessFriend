#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL343.h>

#define ADXL343_SCK 13
#define ADXL343_MISO 12
#define ADXL343_MOSI 11
#define ADXL343_CS 10

/* Assign a unique ID to this sensor at the same time */
/* Uncomment following line for default Wire bus      */
Adafruit_ADXL343 accel = Adafruit_ADXL343(12345);

/* NeoTrellis M4, etc.                    */
/* Uncomment following line for Wire1 bus */
//Adafruit_ADXL343 accel = Adafruit_ADXL343(12345, &Wire1);

/* Uncomment for software SPI */
//Adafruit_ADXL343 accel = Adafruit_ADXL343(ADXL343_SCK, ADXL343_MISO, ADXL343_MOSI, ADXL343_CS, 12345);

/* Uncomment for hardware SPI */
//Adafruit_ADXL343 accel = Adafruit_ADXL343(ADXL343_CS, &SPI, 12345);


// Step detection variables
float previousMagnitude = 0;
float currentMagnitude = 0;
bool peakDetected = false;
unsigned long lastStepTime = 0;
int stepCount = 0;

// Thresholds - adjust these based on your needs
const float STEP_THRESHOLD = 1.2;  // Minimum acceleration change for step
const float NOISE_THRESHOLD = 0.3; // Filter out small movements
const unsigned long MIN_STEP_INTERVAL = 300; // Minimum time between steps (ms)

// Simple moving average for smoothing (optional)
const int SAMPLE_SIZE = 3;
float magnitudeBuffer[SAMPLE_SIZE];
int bufferIndex = 0;
bool bufferFilled = false;

void displayDataRate(void)
{
  Serial.print  ("Data Rate:    ");

  switch(accel.getDataRate())
  {
    case ADXL343_DATARATE_3200_HZ:
      Serial.print  ("3200 ");
      break;
    case ADXL343_DATARATE_1600_HZ:
      Serial.print  ("1600 ");
      break;
    case ADXL343_DATARATE_800_HZ:
      Serial.print  ("800 ");
      break;
    case ADXL343_DATARATE_400_HZ:
      Serial.print  ("400 ");
      break;
    case ADXL343_DATARATE_200_HZ:
      Serial.print  ("200 ");
      break;
    case ADXL343_DATARATE_100_HZ:
      Serial.print  ("100 ");
      break;
    case ADXL343_DATARATE_50_HZ:
      Serial.print  ("50 ");
      break;
    case ADXL343_DATARATE_25_HZ:
      Serial.print  ("25 ");
      break;
    case ADXL343_DATARATE_12_5_HZ:
      Serial.print  ("12.5 ");
      break;
    case ADXL343_DATARATE_6_25HZ:
      Serial.print  ("6.25 ");
      break;
    case ADXL343_DATARATE_3_13_HZ:
      Serial.print  ("3.13 ");
      break;
    case ADXL343_DATARATE_1_56_HZ:
      Serial.print  ("1.56 ");
      break;
    case ADXL343_DATARATE_0_78_HZ:
      Serial.print  ("0.78 ");
      break;
    case ADXL343_DATARATE_0_39_HZ:
      Serial.print  ("0.39 ");
      break;
    case ADXL343_DATARATE_0_20_HZ:
      Serial.print  ("0.20 ");
      break;
    case ADXL343_DATARATE_0_10_HZ:
      Serial.print  ("0.10 ");
      break;
    default:
      Serial.print  ("???? ");
      break;
  }
  Serial.println(" Hz");
}

void displayRange(void)
{
  Serial.print  ("Range:         +/- ");

  switch(accel.getRange())
  {
    case ADXL343_RANGE_16_G:
      Serial.print  ("16 ");
      break;
    case ADXL343_RANGE_8_G:
      Serial.print  ("8 ");
      break;
    case ADXL343_RANGE_4_G:
      Serial.print  ("4 ");
      break;
    case ADXL343_RANGE_2_G:
      Serial.print  ("2 ");
      break;
    default:
      Serial.print  ("?? ");
      break;
  }
  Serial.println(" g");
}

void setup_accel(void)
{
  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL343 ... check your connections */
    Serial.println("Ooops, no ADXL343 detected ... Check your wiring!");
    while(1);
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL343_RANGE_16_G);
  // accel.setRange(ADXL343_RANGE_8_G);
  // accel.setRange(ADXL343_RANGE_4_G);
  // accel.setRange(ADXL343_RANGE_2_G);

  /* Display some basic information on this sensor */
  accel.printSensorDetails();
  displayDataRate();
  displayRange();
  Serial.println("");
}

float calculateMagnitude(float x, float y, float z) {
  return sqrt(x*x + y*y + z*z);
}

float getSmoothedMagnitude(float magnitude) {
  // Add new reading to circular buffer
  magnitudeBuffer[bufferIndex] = magnitude;
  bufferIndex = (bufferIndex + 1) % SAMPLE_SIZE;
  
  if(!bufferFilled && bufferIndex == 0) {
    bufferFilled = true;
  }
  
  // Calculate average
  float sum = 0;
  int count = bufferFilled ? SAMPLE_SIZE : bufferIndex;
  for(int i = 0; i < count; i++) {
    sum += magnitudeBuffer[i];
  }
  
  return sum / count;
}

bool detectStep(float magnitude) {
  static bool lookingForMax = true;
  static bool lookingForMin = false;
  static float maxValue = 0;
  static float minValue = 0;
  
  unsigned long currentTime = millis();
  
  // Prevent too frequent step detection
  if(currentTime - lastStepTime < MIN_STEP_INTERVAL) {
    return false;
  }
  
  if(lookingForMax) {
    if(magnitude > maxValue) {
      maxValue = magnitude;
    }
    else if(magnitude < maxValue - NOISE_THRESHOLD) {
      // Found peak, now look for valley
      lookingForMax = false;
      lookingForMin = true;
      minValue = magnitude;
    }
  }
  else if(lookingForMin) {
    if(magnitude < minValue) {
      minValue = magnitude;
    }
    else if(magnitude > minValue + NOISE_THRESHOLD) {
      // Found valley, check if the peak-to-valley difference indicates a step
      float peakToValley = maxValue - minValue;
      
      if(peakToValley > STEP_THRESHOLD) {
        // Step detected!
        lookingForMax = true;
        lookingForMin = false;
        maxValue = magnitude;
        lastStepTime = currentTime;
        return true;
      }
      else {
        // Not a significant enough change, reset and continue looking
        lookingForMax = true;
        lookingForMin = false;
        maxValue = magnitude;
      }
    }
  }
  
  return false;
}

bool accel_loop(void) {
  /* Get a new sensor event */
  sensors_event_t event;
  accel.getEvent(&event);
  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;
  
  // Calculate magnitude of acceleration vector
  float magnitude = calculateMagnitude(x, y, z);
  
  // Optional: Apply smoothing filter
  float smoothedMagnitude = getSmoothedMagnitude(magnitude);
  
  // Detect steps using the smoothed magnitude
  if(detectStep(smoothedMagnitude)) {
    stepCount++;
    Serial.print("Step detected! Total steps: ");
    Serial.println(stepCount);
    return true;
    
    // Optional: Add a brief LED flash or beep here
    // digitalWrite(LED_PIN, HIGH);
    // delay(100);
    // digitalWrite(LED_PIN, LOW);
  }
  return false;
  // Optional: Print raw data for debugging
  // /*
  // Serial.print("X: "); Serial.print(x);
  // Serial.print(" Y: "); Serial.print(y);
  // Serial.print(" Z: "); Serial.print(z);
  // Serial.print(" Mag: "); Serial.print(magnitude);
  // Serial.print(" Smoothed: "); Serial.println(smoothedMagnitude);
  // */
  
}
