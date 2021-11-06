//--------------------------------------------------------------------------------
// 九州新幹線 800系用　流れ星新幹線室内灯
// [shootingstar.ino]
// Copyright (c) 2021 Ayanosuke(Maison de DCC)
//
// http://maison-dcc.sblo.jp/ http://dcc.client.jp/ http://ayabu.blog.shinobi.jp/
// https://twitter.com/masashi_214
//
// DCC電子工作連合のメンバーです
// https://desktopstation.net/tmi/ https://desktopstation.net/bb/index.php
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//
// 2021/08/14 流れ星動いたバージョン
// 2021/08/14 SW設定により流れ星と白色室内灯切り替え追加
// 2021/09/25 SW設定は色がわかる方向に変更
// 2021/09/25 車両位置による色の開始アドレスを[#define CarNum 1]で変更できるようにした。
//
// PIN_A0 UPDI
// PIN_A1 WS2812C-2000 DO
// PIN_A2 SW NO
// PIN_A3 SW NC
// PIN_A6 JP2
// PIN_A7 JP2
//--------------------------------------------------------------------------------

#include <arduino.h>
#include "Adafruit_NeoPixel.h"
#define MAXLED 9                // プリント基板に実装しているWS2812C-2020の使用数
#define LedMaxLevel 50
#define tblMax LedMaxLevel*3*2
#define WAIT_TIMER 50
#define LedStep 5
#define CarNum 1    // n両目

const char tbl[300]={50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50};
const int StartTbl[7][3]={ {  0,  0,  0},     // 車両による点灯開始アドレステーブル
                            {  0,100,200},    // 1両目
                            { 50,150,250},    // 2両目
                            {100,200,  0},    // 3両目
                            {150,250, 50},    // 4両目
                            {200,  0,100},    // 5両目
                            {250, 50,200}};   // 6両目
                      
Adafruit_NeoPixel pixels(MAXLED, PIN_A1, NEO_GRB + NEO_KHZ800);

unsigned long gTimer = 0;
char Rotate = 0;            // 色が変わる方向指定(右回り、左回り)

//各色のtbl[]に対するシフト量を計算 設計ノート参照
//tbl[]は一色分しかないので、シフトさせてRGBを再現
int rcnt = StartTbl[CarNum][0];
int bcnt = StartTbl[CarNum][1];
int gcnt = StartTbl[CarNum][2];

//int rcnt = 0;
//int bcnt = LedMaxLevel*2;
//int gcnt = bcnt*2;

void setup() {
  pixels.begin();

  pinMode(PIN_A2,INPUT_PULLUP); // SWのポートをpullupに設定
  if(digitalRead(PIN_A2)){  // sw 2側
    Rotate = 1;
  } else {                  // sw 1側
    Rotate = 0;
  } 
  
  gTimer = millis();
}

void loop() {
  char ch = 0;
  int lrcnt = 0;
  int lgcnt = 0;
  int lbcnt = 0;
  
  if ( (millis() - gTimer) >= WAIT_TIMER){  // 時間管理
    gTimer = millis();

    //色の移り変わり処理(tbl[]はリングバッファとして使用するので最終まで来たら0に再設定する)
    rcnt++;
    if(rcnt>tblMax-1){
      rcnt = 0;
    }
    gcnt++;
    if(gcnt>tblMax-1){
      gcnt = 0;
    }
    bcnt++;
    if(bcnt>tblMax-1){
      bcnt = 0;
    }    
    
    for(ch = 0; ch < 9 ; ch++){ // WS2812C-2010 9個分セット
      lrcnt = cal(rcnt,ch*LedStep);  // cn*n 次のchのtbl[]参照の飛び幅
      lgcnt = cal(gcnt,ch*LedStep);
      lbcnt = cal(bcnt,ch*LedStep);
      if(Rotate == 1 ){
        pixels.setPixelColor(ch, pixels.Color(tbl[lrcnt],tbl[lgcnt],tbl[lbcnt]));
      } else  {
        pixels.setPixelColor(8-ch, pixels.Color(tbl[lrcnt],tbl[lgcnt],tbl[lbcnt]));   // 8-ch にしたのは ch0からch1へ流れるようにしたかった為
      }
    }
    pixels.show();
  }   
 }
 
//--------------------------------------------------------------------------------
// 各ポジションLED用tbl[]シフト量を計算
// tblMaxを超えたら0から再計算 
//--------------------------------------------------------------------------------
int cal(int pos, int addpos){
  int lvar =0;

  lvar = pos + addpos;
  if(lvar >= tblMax){
    lvar = lvar - tblMax;
  }
  return lvar;
}
