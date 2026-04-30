/*
* 3D rotating cube with basic depth shading
* and frame counter + frame timing
*/

#include <M5Cardputer.h>
#include <M5GFX.h>

LGFX_Sprite sprite(&M5Cardputer.Display);

//frame counter and timing
int frame = 1;
uint32_t lastFrameTime = 0;
uint32_t frameMs = 0;

//cube setup
struct Vec3 {
  float x, y, z;
};

Vec3 vertices[8] = {
  {-1, -1, -1},
  { 1, -1, -1},
  { 1,  1, -1},
  {-1,  1, -1},
  {-1, -1,  1},
  { 1, -1,  1},
  { 1,  1,  1},
  {-1,  1,  1}
};

//rotation functions
Vec3 rotateX(Vec3 v, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return {
    v.x,
    v.y * c - v.z * s,
    v.y * s + v.z * c
  };
}

Vec3 rotateY(Vec3 v, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return {
    v.x * c + v.z * s,
    v.y,
    -v.x * s + v.z * c
  };
}

int edges[12][2] = {
  {0,1},{1,2},{2,3},{3,0},
  {4,5},{5,6},{6,7},{7,4},
  {0,4},{1,5},{2,6},{3,7}
};


// 3D > 2D projection:
void project(Vec3 v, int &x, int &y) {
  float distance = 3.0;
  float scale = 60.0;

  float z = v.z + distance;
  x = (v.x / z) * scale + 120; // center X
  y = (v.y / z) * scale + 67;  // center Y
}

//colour/shading
uint16_t depthColor(float z) {
  float nearZ = 5.0;  // adjusted (cube max ~ 3 + 1) //plus another 1 to reach full brightness at the front?
  float farZ  = 2.0;  // adjusted minimum //lower values make the far side very dark/dissapear

  float t = 1.0f - (z - farZ) / (nearZ - farZ);
  //t = 1.0f - t; //invert depth if you want (makes far side bright, near side dark)
 
  if (t < 0) t = 0;
  if (t > 1) t = 1;

  // optional contrast shaping
  t = t * t;

  uint8_t v = (uint8_t)(t * 255);
  return M5Cardputer.Display.color565(v, v, v);
}

//depth extraction
float depthValue(const Vec3& v) {
  float distance = 3.0;
  return v.z + distance;
}

float angle = 0;

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  // Create a sprite buffer (width 240, height 135 for Cardputer)
  sprite.createSprite(240, 135);

  //display and text setup
  //M5Cardputer.Display.setRotation(1);
  sprite.setTextColor(GREEN);
  sprite.setTextSize(1);
  sprite.setFont(&fonts::FreeMono12pt7b);
}

void loop() {
  M5Cardputer.update();
  //M5Cardputer.Display.clear(); //causes flicker when called in a loop
  //M5Cardputer.Display.fillScreen(BLACK); //also caused flicker. using sprites instead.

  //measure frame time
  uint32_t now = millis();
  frameMs = now - lastFrameTime;
  lastFrameTime = now;

  sprite.fillScreen(TFT_BLACK); //clear the sprite rather than the display

  Vec3 transformed[8];
  int projected[8][2];

  // Rotate and project vertices
  for (int i = 0; i < 8; i++) {
    Vec3 v = vertices[i];

    v = rotateX(v, angle);
    v = rotateY(v, angle * 0.7);

    transformed[i] = v;
    project(v, projected[i][0], projected[i][1]);
  }

  // Draw edges
  for (int i = 0; i < 12; i++) {
  int a = edges[i][0];
  int b = edges[i][1];

  int x1 = projected[a][0];
  int y1 = projected[a][1];
  int x2 = projected[b][0];
  int y2 = projected[b][1];

  float zAvg = (depthValue(transformed[a]) + depthValue(transformed[b])) * 0.5f;

  uint16_t col = depthColor(zAvg);

  sprite.drawLine(x1, y1, x2, y2, col);
  }

  angle += 0.01; //increment the angle for the next frame
  
  //frame counter
  frame++;
  char buf1[32];
  snprintf(buf1, sizeof(buf1), "Frame: %d", frame);
  sprite.drawString(buf1, 0, 0);

  //frame time
  char buf2[32];
  snprintf(buf2, sizeof(buf2), "Frame time: %ums", frameMs);
  sprite.drawString(buf2, 0, 114);

  //delay(2); //add some delay to limit framerate (already runs close to 60fps so no need)
  
  //push the entire sprite to the display
  sprite.pushSprite(0, 0);
}