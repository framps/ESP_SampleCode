const int PWM_period = 50;
const int PWM_steps = 11; 
const int PWM_factor = 2;

class LCD_Dimmer { 

  public:
    LCD_Dimmer(int initialBrightness, BaseType_t xCoreID=0, int priority=1);
    void enable();
    void disable();
    ~LCD_Dimmer();
    int getBrightness();
    int setBrightness(int brigthness);
    int getMaxBrightness();
    int getMinBrightness();

  protected:
    float getPWM(int index);
    int getPeriod();
  
  private:              
    BaseType_t _xCoreID;        // coreid
    int _priority;              // task priority
    int _brightness;               
    int _period;
    TaskHandle_t _taskHandle;   
    void dimmerTask();          // LCD dimming method
    static void s_dimmerTask(void* parameter);  // task callback function
};

// ctor
LCD_Dimmer::LCD_Dimmer(int initialBrightness, BaseType_t xCoreID, int priority) : _xCoreID(xCoreID), _priority(priority), _period(this->getPeriod()), _brightness(this->setBrightness(initialBrightness)) {
    xTaskCreatePinnedToCore(s_dimmerTask, "LCD_Dimmer", 4000, this,_priority, &_taskHandle, _xCoreID);
}

// dtor
LCD_Dimmer::~LCD_Dimmer() {
  this->disable();
  vTaskDelete(_taskHandle);
}

// enable LCD dimming
void LCD_Dimmer::enable() {
  vTaskResume(_taskHandle);
}

// disable LCD dimming
void LCD_Dimmer::disable() {
  vTaskSuspend(_taskHandle);
}

inline float LCD_Dimmer::getPWM(int index) {
  if (index == 0 ) {
    return 0.0;
  } else {   
    double base = pow(PWM_period/PWM_factor, 1.0/(PWM_steps-1.0));
    float pwm = PWM_factor*pow(base,index);
    pwm = round(pwm);
    return pwm;
  }
}

inline int LCD_Dimmer::getPeriod() {
  return PWM_period;
}

inline int LCD_Dimmer::setBrightness(int brightness) {
  if (brightness < this->getMinBrightness()) {
    _brightness=this->getMinBrightness();
  }
  else if (brightness > this->getMaxBrightness()) {
    _brightness=this->getMaxBrightness();
  }
  else {
    _brightness=brightness-1;     
  }
  return _brightness;
}

inline int LCD_Dimmer::getBrightness() {
  return _brightness+1;
}

inline int LCD_Dimmer::getMaxBrightness() {
  return PWM_steps-1;
}

inline int LCD_Dimmer::getMinBrightness() {
  return 0;
}

void LCD_Dimmer::s_dimmerTask(void *parameter) {
  ((LCD_Dimmer*)parameter)->dimmerTask();
}

void LCD_Dimmer::dimmerTask() {

  for (;;) {

    int offTime=this->getPeriod()-this->getPWM(this->getBrightness());
    int onTime=this->getPeriod()-offTime;
    
    Serial.printf("*** Dimmertask *** in core %i - Brightness: %i - On: %i - Off: %i\n",xPortGetCoreID(), this->getBrightness(), onTime, offTime);

    // Serial.println("On");
    delay(onTime);
    // Serial.println("Off");
    delay(offTime);
  }
}

LCD_Dimmer *lcdDimmer;
int i=0;
int inc=1;

void setup() {

  Serial.begin(9600);  
  
  Serial.printf("%s - %i\n","@@@ Setup @@@",xPortGetCoreID());
  
  lcdDimmer= new LCD_Dimmer(0); // start with dark display
}

void loop() {

  Serial.printf("%s - %i - Brighntess: %i\n","@@@ Loop @@@",xPortGetCoreID(), i); 
  delay(100);

  i+=inc;  
  if ( i > lcdDimmer->getMaxBrightness() || i < lcdDimmer->getMinBrightness() ) {
    inc=-inc;
    i+=inc;
  }
  lcdDimmer->setBrightness(i);
  
}
