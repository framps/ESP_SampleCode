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
  Serial.printf("Deleted dimmer in core %i\n",_xCoreID); 
}

// enable LCD dimming
void LCD_Dimmer::enable() {
    Serial.printf("Enabling dimmer in core %i\n",_xCoreID); 
    vTaskResume(_taskHandle);
}

// disable LCD dimming
void LCD_Dimmer::disable() {
  Serial.printf("Disabling dimmer in core %i\n",_xCoreID); 
  vTaskSuspend(_taskHandle);
}

inline float LCD_Dimmer::getPWM(int index) {
  double base = pow(PWM_period/PWM_factor, 1.0/(PWM_steps-1.0));
  float pwm = round(PWM_factor*pow(base,index));
  Serial.printf("PWM: %f\n",pwm);
  return pwm;
}

inline int LCD_Dimmer::getPeriod() {
    return PWM_period;
}

inline int LCD_Dimmer::setBrightness(int brightness) {
    if (brightness < 0) {
      _brightness=0;
    }
    else if (brightness > PWM_steps) {
      _brightness=PWM_steps-1;
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

      Serial.println("On");
      delay(onTime);
      Serial.println("Off");
      delay(offTime);
    }
}

LCD_Dimmer *lcdDimmer;
int i=0;

void setup() {

  Serial.begin(9600);  
  
  Serial.printf("%s - %i\n","@@@ Setup @@@",xPortGetCoreID());
  
  lcdDimmer= new LCD_Dimmer(2);
}

void loop() {

  Serial.printf("%s - %i - Brighntess: \n","@@@ Loop @@@",xPortGetCoreID(), lcdDimmer->getBrightness()); 
  delay(1000);
  lcdDimmer->setBrightness(i);
   if ( i < lcdDimmer->getMaxBrightness() ) {
    i++;
  } else {
    i=0;
  }
  
  delay(100);
}
