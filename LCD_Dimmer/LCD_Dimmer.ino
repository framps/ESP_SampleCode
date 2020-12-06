#include <typeinfo>

#define LED_RED 32
#define LED_GREEN 33

class LCD_Dimmer { 
  public:
    LCD_Dimmer(float dutyCycle, int period, BaseType_t xCoreID=0, int priority=1);
    void begin();
    void enable();
    void disable();
    float getDutyCycle();
    void setDutyCycle(float dutyCycle);
    int getPeriod();
    void setPeriod(int period);
    void set(float dutyCycle, int period);
    ~LCD_Dimmer();
  private:          
    BaseType_t _xCoreID; // coreid
    int _priority;
    int _period;     // ms
    float _dutyCycle; // %
    TaskHandle_t _taskHandle;
    void dimmerTask();
    static void s_dimmerTask(void* parameter);
};

// ctor
LCD_Dimmer::LCD_Dimmer(float dutyCycle, int period, BaseType_t xCoreID, int priority) : _xCoreID(0), _priority(1) {
    _period = period;
    _dutyCycle = dutyCycle;
    _xCoreID = xCoreID;
    _priority=priority;
}
// dtor
LCD_Dimmer::~LCD_Dimmer() {
  this->disable();
  vTaskDelete(_taskHandle);
  Serial.printf("Deleted dimmer in core %i\n",_xCoreID); 
}

// initialize LCD dimming in setup
void LCD_Dimmer::begin() {
    Serial.printf("Beginning dimmer in core %i\n",_xCoreID); 
    xTaskCreatePinnedToCore(s_dimmerTask,"LCD_Dimmer",1000,this,_priority,&_taskHandle,_xCoreID);
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

inline float LCD_Dimmer::getDutyCycle() {
    return _dutyCycle;
}

void LCD_Dimmer::setDutyCycle(float dutyCycle) {
    _dutyCycle=dutyCycle;
}

inline int LCD_Dimmer::getPeriod() {
    return _period;
}

inline void LCD_Dimmer::setPeriod(int period) {
    _period=period;
}

void LCD_Dimmer::s_dimmerTask(void *parameter) {
  ((LCD_Dimmer*)parameter)->dimmerTask();
}

void LCD_Dimmer::dimmerTask() {

  for (;;) {
    Serial.printf("Type: %s\n",typeid(this).name());
    // Serial.printf("DutyCycle: %f, Period: %i\n",this->getDutyCycle(),this->getPeriod());
    Serial.printf("*** Dimmertask *** in core %i\n",xPortGetCoreID());
    yield();
    delay(1000);
  }
}

LCD_Dimmer *lcdDimmer;

void setup() {

  Serial.begin(9600);  
  
  Serial.println("@@@ Setup @@@");

  lcdDimmer= new LCD_Dimmer(50.0,100);
  lcdDimmer->begin();
}

void loop() {
  Serial.println("@@@ Loop @@@");

  lcdDimmer->enable();
  delay(3000);
  lcdDimmer->disable();

  delay(3000);
}
