//+------------------------------------------------------------------+
//|                                                        mymma.mq5 |
//|                        Copyright 2017, MetaQuotes Software Corp. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2017, MetaQuotes Software Corp."
#property link      "https://www.mql5.com"
#property version   "1.00"

#include <Trade\Trade.mqh>

input int    MovingPeriod       = 12;      // Moving Average period
input int    MovingShift        = 0;       // Moving Average shift
//---
int    ExtHandle=0;
int h_ma_fast=0;
int h_ma_slow=0;
bool   ExtHedging=false;
CTrade ExtTrade;
#define MA_MAGIC 123456789
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+

int OnInit(void)
  {
//--- prepare trade class to control positions if hedging mode is active
   ExtHedging=((ENUM_ACCOUNT_MARGIN_MODE)AccountInfoInteger(ACCOUNT_MARGIN_MODE)==ACCOUNT_MARGIN_MODE_RETAIL_HEDGING);
   ExtTrade.SetExpertMagicNumber(MA_MAGIC);
   ExtTrade.SetMarginMode();
//--- Moving Average indicator
   ExtHandle=iMA(_Symbol,_Period,MovingPeriod,MovingShift,MODE_SMA,PRICE_CLOSE);
   h_ma_fast = iMA(_Symbol,_Period,8,0,MODE_SMA,PRICE_CLOSE);
   h_ma_slow = iMA(_Symbol,_Period,20,0,MODE_SMA,PRICE_CLOSE);
   if(ExtHandle==INVALID_HANDLE)
     {
      printf("Error creating MA indicator");
      return(INIT_FAILED);
     }
//--- ok
   return(INIT_SUCCEEDED);
  }
int TradeSignal_01()
  {
//--- 0说明没有信号
   //int sig=0;
   ENUM_ORDER_TYPE sig=WRONG_VALUE;
   double   ma_fast_buffer[];
 double   ma_slow_buffer[];
//--- 检查指标的句柄
   if(h_ma_fast==INVALID_HANDLE)//--- 如果句柄无效
     {
      //--- 再次创建它                                                      
      h_ma_fast=iMA(Symbol(),Period(),8,0,MODE_SMA,PRICE_CLOSE);
      //--- 退出函数
      return(0);
     }
   else //--- 如果句柄有效
     {
      //--- 把指标值复制到数组中
      if(CopyBuffer(h_ma_fast,0,0,3,ma_fast_buffer)<3) //--- 如果数组数据少于所需
         //--- 退出函数
         return(0);
      //--- 设置数组索引方式为倒序                                   
      if(!ArraySetAsSeries(ma_fast_buffer,true))
         //--- 如果索引出错，退出函数
         return(0);
     }

   if(h_ma_slow==INVALID_HANDLE)//--- 如果句柄是无效的
     {
      //--- 再次创建它                                                      
      h_ma_slow=iMA(Symbol(),Period(),20,0,MODE_SMA,PRICE_CLOSE);
      //--- 退出函数
      return(0);
     }
   else //--- 如果句柄有效 
     {
      //--- 把指标值复制到数组中
      if(CopyBuffer(h_ma_slow,0,0,2,ma_slow_buffer)<2) //--- 如果数据少于所需
         //--- 退出函数
         return(0);
      //--- 设置数组索引方式为倒序                                   
      if(!ArraySetAsSeries(ma_slow_buffer,true))
         //--- 如果索引出错，退出函数
         return(0);
     }

//--- 检查条件并设置sig的值
   if(ma_fast_buffer[2]<ma_slow_buffer[1] && ma_fast_buffer[1]>ma_slow_buffer[1])
      sig=ORDER_TYPE_BUY;
   else if(ma_fast_buffer[2]>ma_slow_buffer[1] && ma_fast_buffer[1]<ma_slow_buffer[1])
      sig=ORDER_TYPE_SELL;
  // else sig=0;

if(sig!=WRONG_VALUE)
     {
      if(TerminalInfoInteger(TERMINAL_TRADE_ALLOWED) && Bars(_Symbol,_Period)>100)
         ExtTrade.PositionOpen(_Symbol,sig,0.1,
                               SymbolInfoDouble(_Symbol,sig==ORDER_TYPE_SELL ? SYMBOL_BID:SYMBOL_ASK),
                               0,0);
     }

//--- 返回交易信号
   return(sig);
  }
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
  {
//---
   
  }
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
  {
//---
int s =0;
s=TradeSignal_01();

   
  }
  //+------------------------------------------------------------------+
//| Check for open position conditions                               |
//+------------------------------------------------------------------+
void CheckForOpen(void)
  {
   MqlRates rt[2];
//--- go trading only for first ticks of new bar
   if(CopyRates(_Symbol,_Period,0,2,rt)!=2)
     {
      Print("CopyRates of ",_Symbol," failed, no history");
      return;
     }
   if(rt[1].tick_volume>1)
      return;
//--- get current Moving Average 
   double   ma[1];
   if(CopyBuffer(ExtHandle,0,0,1,ma)!=1)
     {
      Print("CopyBuffer from iMA failed, no data");
      return;
     }
//--- check signals
   ENUM_ORDER_TYPE signal=WRONG_VALUE;

   if(rt[0].open>ma[0] && rt[0].close<ma[0])
      signal=ORDER_TYPE_SELL;    // sell conditions
   else
     {
      if(rt[0].open<ma[0] && rt[0].close>ma[0])
         signal=ORDER_TYPE_BUY;  // buy conditions
     }
//--- additional checking
   if(signal!=WRONG_VALUE)
     {
      if(TerminalInfoInteger(TERMINAL_TRADE_ALLOWED) && Bars(_Symbol,_Period)>100)
         ExtTrade.PositionOpen(_Symbol,signal,0.1,
                               SymbolInfoDouble(_Symbol,signal==ORDER_TYPE_SELL ? SYMBOL_BID:SYMBOL_ASK),
                               0,0);
     }
//---
  }
//+------------------------------------------------------------------+
