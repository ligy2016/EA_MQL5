//+------------------------------------------------------------------+
//|                                                        mymma.mq5 |
//|                        Copyright 2017, MetaQuotes Software Corp. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2017, MetaQuotes Software Corp."
#property link      "https://www.mql5.com"
#property version   "1.00"

#include <Trade\Trade.mqh>
input bool    BuyAllowed = true;      // 
input bool    SellAllowed = true;      // 

input int    FastPeriod = 10;      // FastPeriod
input int    SlowPeriod = 30;       //SlowPeriod
//---
int ExtHandle = 0;
int h_ma_fast = 0;
int h_ma_slow = 0;
bool   ExtHedging = false;
CTrade ExtTrade;
#define MA_MAGIC 123456789
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+

int OnInit(void)
{
	//--- prepare trade class to control positions if hedging mode is active
	ExtHedging = ((ENUM_ACCOUNT_MARGIN_MODE)AccountInfoInteger(ACCOUNT_MARGIN_MODE) == ACCOUNT_MARGIN_MODE_RETAIL_HEDGING);
	ExtTrade.SetExpertMagicNumber(MA_MAGIC);
	ExtTrade.SetMarginMode();
	//--- Moving Average indicator
	   //ExtHandle=iMA(_Symbol,_Period,MovingPeriod,MovingShift,MODE_SMA,PRICE_CLOSE);
	h_ma_fast = iMA(_Symbol, _Period, FastPeriod, 0, MODE_SMA, PRICE_CLOSE);
	h_ma_slow = iMA(_Symbol, _Period, SlowPeriod, 0, MODE_SMA, PRICE_CLOSE);
	if (h_ma_fast == INVALID_HANDLE || h_ma_slow == INVALID_HANDLE)
	{
		printf("Error creating MA indicator");
		return(INIT_FAILED);
	}
	//--- ok

	EventSetTimer(15);
	return(INIT_SUCCEEDED);
}
void OnTimer()
{
	//PrintFormat("5");
		if (SelectPosition())
		CheckForClose();
	else
		CheckForOpen();
}
int CheckForOpen()
{
	//--- 0说明没有信号
	   //int sig=0;
	ENUM_ORDER_TYPE sig = WRONG_VALUE;
	double   ma_fast_buffer[];
	double   ma_slow_buffer[];
	//--- 检查指标的句柄
	if (h_ma_fast == INVALID_HANDLE)//--- 如果句柄无效
	{
		//--- 再次创建它                                                      
		h_ma_fast = iMA(Symbol(), Period(), 8, 0, MODE_SMA, PRICE_CLOSE);
		//--- 退出函数
		return(0);
	}
	else //--- 如果句柄有效
	{
		//--- 把指标值复制到数组中
		if (CopyBuffer(h_ma_fast, 0, 0, 3, ma_fast_buffer) < 3) //--- 如果数组数据少于所需
		   //--- 退出函数
			return(0);
		//--- 设置数组索引方式为倒序                                   
		if (!ArraySetAsSeries(ma_fast_buffer, true))
			//--- 如果索引出错，退出函数
			return(0);
	}

	if (h_ma_slow == INVALID_HANDLE)//--- 如果句柄是无效的
	{
		//--- 再次创建它                                                      
		h_ma_slow = iMA(Symbol(), Period(), 20, 0, MODE_SMA, PRICE_CLOSE);
		//--- 退出函数
		return(0);
	}
	else //--- 如果句柄有效 
	{
		//--- 把指标值复制到数组中
		if (CopyBuffer(h_ma_slow, 0, 0, 3, ma_slow_buffer) < 3) //--- 如果数据少于所需
		   //--- 退出函数
			return(0);
		//--- 设置数组索引方式为倒序                                   
		if (!ArraySetAsSeries(ma_slow_buffer, true))
			//--- 如果索引出错，退出函数
			return(0);
	}

	//--- 检查条件并设置sig的值
	if (BuyAllowed && ma_fast_buffer[2] <= ma_slow_buffer[2] && ma_fast_buffer[1] > ma_slow_buffer[1])
		sig = ORDER_TYPE_BUY;
	else if (SellAllowed && ma_fast_buffer[2] >= ma_slow_buffer[2] && ma_fast_buffer[1] < ma_slow_buffer[1])
		sig = ORDER_TYPE_SELL;
	// else sig=0;

	if (sig != WRONG_VALUE)
	{
		if (TerminalInfoInteger(TERMINAL_TRADE_ALLOWED) && Bars(_Symbol, _Period) > 100)
			ExtTrade.PositionOpen(_Symbol, sig, 0.1, SymbolInfoDouble(_Symbol, sig == ORDER_TYPE_SELL ? SYMBOL_BID : SYMBOL_ASK), 0, 0);
	}

	//--- 返回交易信号
	return 0;
}

bool SelectPosition()
{
	bool res = false;
	//---
	if (ExtHedging)
	{
		uint total = PositionsTotal();
		for (uint i = 0; i < total; i++)
		{
			string position_symbol = PositionGetSymbol(i);
			if (_Symbol == position_symbol && MA_MAGIC == PositionGetInteger(POSITION_MAGIC))
			{
				res = true;
				break;
			}
		}
	}
	else
		res = PositionSelect(_Symbol);
	//---
	return(res);
}

//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
	//---

}

void CheckForClose(void)
{

	double   ma_fast_buffer[];
	double   ma_slow_buffer[];
	if (CopyBuffer(h_ma_fast, 0, 0, 2, ma_fast_buffer) < 2) //--- 如果数组数据少于所需
		 //--- 退出函数
		return;
	//--- 设置数组索引方式为倒序                                   
	if (!ArraySetAsSeries(ma_fast_buffer, true))
		//--- 如果索引出错，退出函数
		return;


	if (CopyBuffer(h_ma_slow, 0, 0, 2, ma_slow_buffer) < 2) //--- 如果数据少于所需
	   //--- 退出函数
		return;
	//--- 设置数组索引方式为倒序                                   
	if (!ArraySetAsSeries(ma_slow_buffer, true))
		//--- 如果索引出错，退出函数
		return;



	//--- positions already selected before
	bool signal = false;
	long type = PositionGetInteger(POSITION_TYPE);


	if (type == (long)POSITION_TYPE_SELL && ma_fast_buffer[1] > ma_slow_buffer[1])//快速均线上穿
		signal = true;
	else if (type == (long)POSITION_TYPE_BUY   &&  ma_fast_buffer[1] < ma_slow_buffer[1])//快速均线下穿
		signal = true;

	//--- additional checking
	if (signal)
	{
		if (TerminalInfoInteger(TERMINAL_TRADE_ALLOWED) && Bars(_Symbol, _Period) > 100)
			ExtTrade.PositionClose(_Symbol, 3);
	}
	//PrintFormat("type:%d,TERMINAL_TRADE_ALLOWED:%d _Symbol:%s _Period:%d  bars:%d",type,TerminalInfoInteger(TERMINAL_TRADE_ALLOWED),_Symbol, _Period,Bars(_Symbol, _Period) );
	PrintFormat("ma_fast_buffer[1]:%G \nma_slow_buffer[1]:%G",ma_fast_buffer[1],ma_slow_buffer[1]);
	//PrintFormat("ma_fast_buffer[0]:%G \nma_fast_buffer[1]:%G\nma_fast_buffer[2]:%G",ma_fast_buffer[0],ma_fast_buffer[1],ma_fast_buffer[2]);
	//Comment(StringFormat("ma_fast_buffer[1] = %G\n ma_slow_buffer[1] = %G\n", ma_fast_buffer[1] , ma_slow_buffer[1] ));
 //---
}
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
{
	//---
	//--- 
	double Ask, Bid;
	int Spread;
	Ask = SymbolInfoDouble(Symbol(), SYMBOL_ASK);
	Bid = SymbolInfoDouble(Symbol(), SYMBOL_BID);
	Spread = SymbolInfoInteger(Symbol(), SYMBOL_SPREAD);
	//--- 3行输出值 
	Comment(StringFormat("Ask = %G\nBid = %G\nSpread = %d\n fast = %d\n slow = %d\n SellAllowed = %d\n,BuyAllowed = %d", Ask, Bid, Spread, FastPeriod, SlowPeriod, SellAllowed, BuyAllowed));
}
//+------------------------------------------------------------------+
//| Check for open position conditions                               |
//+------------------------------------------------------------------+

