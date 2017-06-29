//+------------------------------------------------------------------+
//|                                                         test.mq5 |
//|                        Copyright 2017, MetaQuotes Software Corp. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2017, MetaQuotes Software Corp."
#property link      "https://www.mql5.com"
#property version   "1.00"
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
  {
//---
   
//---
   return(INIT_SUCCEEDED);
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
   
  }
//+------------------------------------------------------------------+
void OnStart() 
  { 
//--- 
   MqlRates rates[]; 
   ArraySetAsSeries(rates,true); 
   int copied=CopyRates(Symbol(),0,0,100,rates); 
   if(copied>0) 
     { 
      Print("Bars copied: "+copied); 
      string format="open = %G, high = %G, low = %G, close = %G, volume = %d"; 
      string out; 
      int size=fmin(copied,10); 
      for(int i=0;i<size;i++) 
        { 
         out=i+":"+TimeToString(rates[i].time); 
         out=out+" "+StringFormat(format, 
                                  rates[i].open, 
                                  rates[i].high, 
                                  rates[i].low, 
                                  rates[i].close, 
                                  rates[i].tick_volume); 
         Print(out); 
        } 
     } 
   else Print("Failed to get history data for the symbol ",Symbol()); 
  }