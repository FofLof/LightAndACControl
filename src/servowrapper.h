#ifndef SERVO_WRAPPER_H
    #define SERVO_WRAPPER_H
    #include <ESP32Servo.h>
    
    class ServoWrapper {
        public:
            ServoWrapper(int port, int upPos, int downPos, int defaultPos);
            ServoWrapper();
            void setToUp();
            void setToDown();
            void invert();
            void write(int position);
                
        private:
            int port = 0;
            int upPos = 0;
            int downPos = 0;
            Servo servo;
            bool position = false;
    };

#endif
   