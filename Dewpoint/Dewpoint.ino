/* 
 *
 * Dew point, absolute moisture & vapor pressure calculation from temperature & humidity 
 *  
 * Formula source: https://community.hiveeyes.org/t/dew-point-absolute-moisture-vapor-pressure-from-temperature-humidity-with-influxdb-flux/2934/1
 * Dew point calculator: https://www.omnicalculator.com/physics/dew-point
 * 
 * Variables:
 * 
 * r = relative humidity [%]
 * T = temperature in °C
 * TK = temperatur in Kelvin
 * TD = dew point temperature in °C
 * DD = vapor pressure in hPa
 * SDD = saturation vapor pressure in hPa
 * AF = absolute moisture in g/m^3
 *
*/

/*
#######################################################################################################################
#
#    Copyright (c) 2021 framp at linux-tips-and-tricks dot de
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#######################################################################################################################
*/

// Constants
const float a[2] = { 7.5, 7.6 };
const float b[2] = { 237.3, 240.7 };  // Constants for T >= 0 and T < 0
const float Rstar = 8314.3;           // universal gas constant in J(Kmol*K)
const float mw = 18.016;              // molecular weight of steam in kg/kmol
const float OffsetKelvin = 273.15;

float TK(float T) {
  return T+OffsetKelvin;
}  

float SDD(float T){                                // saturation vapor pressure 
  int i=(T >= 0 ? 0 : 1);
  return 6.1078*std::pow(10,((a[i]*T)/(b[i]+T)));
}

float DD(float r, float T) {                       // vapor pressure
  return r/100.0*SDD(T);
}  

float TD(float r, float T) {                       // dew point
  int i=(T >= 0 ? 0 : 1);
  float v=std::log10(DD(r,T)/6.1078);
  return b[i]*v/(a[i]-v);
}  

float AF(float r, float T) {                       // absolute moisture
  return 100000.0*mw/Rstar*DD(r,T)/TK(T);
}  

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

}

struct sample {
  float T;
  float r;
};

const sample Samples[] = { 
              {30, 10},
              {30, 40},
              {30, 70},
              {20, 10},
              {20, 40},
              {20, 70},
              {10, 10},
              {10, 40},
              {10, 70},
              {0, 10},
              {0, 40},
              {0, 70},
              {-10, 10},
              {-10, 40},
              {-10, 70}
              };

void loop() {
  // put your main code here, to run repeatedly:

    for (int i=0; i<sizeof(Samples)/sizeof(sample); i++) {

      if ( i % 3 == 0 ) {
        Serial.println();
      }

      float T = Samples[i].T;
      float r = Samples[i].r;

      Serial.printf("T:  %7.2f °C\n", T );
      Serial.printf("RH: %7.2f %%\n", r );
      Serial.printf("TP: %7.2f °C\n", TD(r,T) );
      Serial.printf("AF: %7.2f g/m^3\n", AF(r,T) );

    }

    delay(60000);
}
