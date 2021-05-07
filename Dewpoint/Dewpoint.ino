/* 
 *
 * Dew point sample sketch
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

#include "Dewpoint.h"

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
