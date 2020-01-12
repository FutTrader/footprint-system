#include "sierrachart.h"

SCDLLName("Footprint Reversal System")

/* Objective: On a subgraph below the FP, draw a bar with an audible alert when the follow conditions are met
			  1. Relative volume is greater as per VolumeBarDiff from NumberBars2 study
			  2. No unfinished auctions 
			  3. Diagonal buy imbalance - User set input
			  4. Footprint bar volume profile - POC withing upper or lower bounds of bar

	Input: 	  1. Number Bars Calculated Study with SG14 VolBarDiff Array
	Output:   Draw bar with audible alert after triggering all 4 conditions
*/

SCSFExport scsf_FootprintReversalSystem(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Buy = sc.Subgraph[0];
	SCSubgraphRef Sell = sc.Subgraph[1];

	SCInputRef Input_LongSignal = sc.Input[0];
	SCInputRef Input_ShortSignal = sc.Input[1];
	SCInputRef Input_AlertNumber = sc.Input[2];
	SCInputRef Input_Study1 = sc.Input[3];	//Must select NumberBars2
	SCInputRef Input_Study1Subgraph = sc.Input[4]; // Must select SG14 VolBarDiff Array
	SCInputRef Input_FillToTopBottomBar = sc.Input[5];
	SCInputRef Input_PercentageThreshold = sc.Input[6];
	SCInputRef Input_AllowZeroValueCompares = sc.Input[7];
	SCInputRef Input_DivideByZeroAction = sc.Input[8];
	SCInputRef Input_PBBarLimit = sc.Input[9];

	if (sc.SetDefaults)
	{
		// Set the configuration and defaults
		sc.GraphName = "Footprint Reversal System Study";

		sc.GraphRegion = 0;
		sc.AutoLoop = 0;//Manual looping
		sc.ValueFormat = sc.BaseGraphValueFormat;

		// During development set this flag to 1, so the DLL can be rebuilt without restarting Sierra Chart. When development is completed, set it to 0 to improve performance.
		sc.FreeDLL = 0;
		sc.MaintainVolumeAtPriceData = 1;  // true
		sc.CalculationPrecedence = LOW_PREC_LEVEL;

		Buy.Name = "Buy";
		Buy.PrimaryColor = RGB(0, 255, 0);	// green
		Buy.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
		Buy.DrawZeros = 0;  //Set to 0 to disable drawing of zero values. Without it there will be a continuous line drawn.

		Sell.Name = "Sell";
		Sell.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
		Sell.PrimaryColor = RGB(255, 0, 0);	// red
		Sell.DrawZeros = 0; //Set to 0 to disable drawing of zero values. Without it there will be a continuous line drawn.

		Input_LongSignal.Name = "Alert Buy Signals";
		Input_LongSignal.SetYesNo(true);

		Input_ShortSignal.Name = "Alert Sell Signals";
		Input_ShortSignal.SetYesNo(true);

		Input_AlertNumber.Name = "Reversal Alert Number";
		Input_AlertNumber.SetAlertSoundNumber(3);

		Input_Study1.Name = "Input NumberBars2";
		Input_Study1.SetStudyID(0);

		Input_Study1Subgraph.Name = "Relative Volume Study (SG14): ";
		Input_Study1Subgraph.SetSubgraphIndex(0);

		Input_FillToTopBottomBar.Name = "Draw Style Mode to Top or Bottom of Bar";
		Input_FillToTopBottomBar.SetCustomInputStrings("Top ; Bottom");
		Input_FillToTopBottomBar.SetCustomInputIndex(0);

		Input_PercentageThreshold.Name = "Buy/Sell Imabalance Percentage Threshold";
		Input_PercentageThreshold.SetInt(400);

		Input_AllowZeroValueCompares.Name = "Enable Zero Bid/Ask Compares";
		Input_AllowZeroValueCompares.SetYesNo(0);

		Input_DivideByZeroAction.Name = "Zero Value Compare Action";
		Input_DivideByZeroAction.SetCustomInputStrings("Set 0 to 1;Set Percentage to +/- 1000%");
		Input_DivideByZeroAction.SetCustomInputIndex(0);

		Input_PBBarLimit.Name = "Number of TICKS from High/Low for PB Reversal";
		Input_PBBarLimit.SetInt(4);

		sc.ValueFormat = VALUEFORMAT_INHERITED;

		return;
	}

	// Pre-initializations

	//This is an indication that the volume at price data does not exist
	if ((int)sc.VolumeAtPriceForBars->GetNumberOfBars() < sc.ArraySize)
		return;

	bool LongSignal = Input_LongSignal.GetYesNo();
	bool ShortSignal = Input_ShortSignal.GetYesNo();
	bool EnableAlerts = sc.IsFullRecalculation == 0 && !sc.DownloadingHistoricalData;

	SCFloatArray RelativeVolumeArray;
	sc.GetStudyArrayUsingID(Input_Study1.GetStudyID(), Input_Study1Subgraph.GetSubgraphIndex(), RelativeVolumeArray);

	for (int BarIndex = sc.UpdateStartIndex; BarIndex < sc.ArraySize; BarIndex++)
	{
		/* This is used further down to position a buy or sell arrow below or above a price bar so it does not touch exactly the low or high.*/
		float Offset = (sc.High[BarIndex] - sc.Low[BarIndex]) * (15 * 0.01f);
		

		//Reset all subgraph values
		for (int SubgraphIndex = 0; SubgraphIndex < SC_SUBGRAPHS_AVAILABLE - 2; ++SubgraphIndex)
			sc.Subgraph[SubgraphIndex].Data[BarIndex] = 0;

		int NumberOfPricesAtBarIndex = sc.VolumeAtPriceForBars->GetSizeAtBarIndex(BarIndex);

		//************************ INITALIZING CHECKS ******************************************************
		bool RelVol = false;
		bool NoUnfinishedBusiness = false;
		bool BuyImbalance = false;
		bool SellImbalance = false;
		bool BReversal = false; //long reversal
		bool PReversal = false; //short reversal
		
		//************************ RELATIVE VOLUME CHECK FROM NUMBERS CALCULATED STUDY *********************
		float Bar1 = RelativeVolumeArray[BarIndex];
		if (Bar1 > 0) { RelVol = true; }

		//************************ LOOPING THROUGH RANGE BAR TICKS TO CHECK VOLUME STUDIES *****************
		for (int PriceIndex = 0; PriceIndex < NumberOfPricesAtBarIndex; ++PriceIndex)
		{
			const s_VolumeAtPriceV2* p_VolumeAtPrice = NULL;

			if (!sc.VolumeAtPriceForBars->GetVAPElementAtIndex(BarIndex, PriceIndex, &p_VolumeAtPrice))
				continue;

			const s_VolumeAtPriceV2* p_NextVolumeAtPrice = NULL;

			if (PriceIndex < NumberOfPricesAtBarIndex - 1)
				sc.VolumeAtPriceForBars->GetVAPElementAtIndex(BarIndex, PriceIndex + 1, &p_NextVolumeAtPrice);

			//********************************** UNFINISHED AUCTION
			if (LongSignal && (PriceIndex == 0)) // Long only
			{
				if (p_VolumeAtPrice->AskVolume == 0) { NoUnfinishedBusiness = true; }
			}
			else if (ShortSignal && (PriceIndex == NumberOfPricesAtBarIndex - 1))
			{
				if (p_VolumeAtPrice->BidVolume == 0) { NoUnfinishedBusiness = true; }
			}

			//********************************** ASK VOL / BID VOL DIAGONAL RATIO
			bool AllowZeroValueComparesSetting = Input_AllowZeroValueCompares.GetYesNo();
			unsigned int DivideByZeroActionIndex = Input_DivideByZeroAction.GetIndex();
			int AskVolumeBidVolumeRatioPercent = 0;

			if (p_NextVolumeAtPrice != NULL)
			{
				int PercentThresholdSigned = Input_PercentageThreshold.GetInt();

				if (LongSignal && (p_NextVolumeAtPrice->AskVolume > p_VolumeAtPrice->BidVolume) && (p_VolumeAtPrice->BidVolume > 0 || AllowZeroValueComparesSetting))
				// Buy Imbalance and BidVol != 0 or ZeroValCompare not allowed
				{
					if (p_VolumeAtPrice->BidVolume == 0 && DivideByZeroActionIndex == 0)
						AskVolumeBidVolumeRatioPercent = (p_NextVolumeAtPrice->AskVolume / 1) * 100;
					else if (p_VolumeAtPrice->BidVolume == 0 && DivideByZeroActionIndex == 1)
						AskVolumeBidVolumeRatioPercent = 1000;
					else
						AskVolumeBidVolumeRatioPercent = sc.Round(((float)p_NextVolumeAtPrice->AskVolume / p_VolumeAtPrice->BidVolume) * 100);

					//Comparing with Threshold
					if (PercentThresholdSigned > 0 && (AskVolumeBidVolumeRatioPercent >= PercentThresholdSigned))
					{
						BuyImbalance = true;
					}
				}
				
				if (ShortSignal && (p_VolumeAtPrice->BidVolume > p_NextVolumeAtPrice->AskVolume) && (p_NextVolumeAtPrice->AskVolume > 0 || AllowZeroValueComparesSetting))
				// Sell Imbalance and AskVol != 0 or ZeroValCompare not allowed
				{
					if (p_NextVolumeAtPrice->AskVolume == 0 && DivideByZeroActionIndex == 0)
						AskVolumeBidVolumeRatioPercent = (p_VolumeAtPrice->BidVolume / 1) * 100;
					else if (p_NextVolumeAtPrice->AskVolume == 0 && DivideByZeroActionIndex == 1)
						AskVolumeBidVolumeRatioPercent = 1000;
					else
						AskVolumeBidVolumeRatioPercent = sc.Round(((float)p_VolumeAtPrice->BidVolume / p_NextVolumeAtPrice->AskVolume) * 100);

					//Comparing with Threshold
					if (PercentThresholdSigned > 0 && (AskVolumeBidVolumeRatioPercent >= PercentThresholdSigned))
					{
						SellImbalance = true;
					}
				}	
			}
		}

		//************************ P-B VOL PROFILE REVERSAL ************************************************
		s_VolumeAtPriceV2 TickPOC;
		sc.GetPointOfControlPriceVolumeForBar(BarIndex, TickPOC);
		float PricePOC = TickPOC.PriceInTicks * sc.TickSize;
		unsigned int BarLimit = Input_PBBarLimit.GetInt();

		/* Check PBBarLimit User input to make sure it's within the bounds of the bar size, if not set to default */		
		if (BarLimit >= NumberOfPricesAtBarIndex)
		{
			BarLimit = 1;
		}

		if (PricePOC <= (sc.High[BarIndex] - (sc.TickSize * BarLimit))) //Compares to the Upper Price Limit for a long reversal
		{
			BReversal = true;
		}
		else if (PricePOC >= (sc.Low[BarIndex] + (sc.TickSize * BarLimit))) //Compares to the Lower Price Limit for a short reversal
		{
			PReversal = true;
		}

		//********************************** DRAWING TRANSPARENT BOXES AND ALERTS
		unsigned int FillTopBottomIndex = Input_FillToTopBottomBar.GetIndex();

		if (RelVol && NoUnfinishedBusiness && BuyImbalance && BReversal) //long signal
		{
			if (FillTopBottomIndex == 0)
			{
				Buy[BarIndex] = sc.High[BarIndex] + Offset;
				Sell[BarIndex] = 0;
			}
			else
			{
				Buy[BarIndex] = sc.Low[BarIndex] - Offset;
				Sell[BarIndex] = 0;
			}
			
			if (EnableAlerts && Input_AlertNumber.GetAlertSoundNumber() > 0 && BarIndex == sc.ArraySize - 1)
			{
				sc.SetAlert(Input_AlertNumber.GetAlertSoundNumber() - 1, "B Reversal triggered");
			}
		}
		else if (RelVol && NoUnfinishedBusiness && SellImbalance && PReversal) //short signal
		{
			if (FillTopBottomIndex == 0)
			{
				Buy[BarIndex] = 0;
				Sell[BarIndex] = sc.High[BarIndex] + Offset;
			}
			else
			{
				Buy[BarIndex] = 0;
				Sell[BarIndex] = sc.Low[BarIndex] - Offset;
			}
			
			if (EnableAlerts && Input_AlertNumber.GetAlertSoundNumber() > 0 && BarIndex == sc.ArraySize - 1)
			{
				sc.SetAlert(Input_AlertNumber.GetAlertSoundNumber() - 1, "P Reversal triggered");
			}
		}
		else
		{
			Buy[BarIndex] = 0;
			Sell[BarIndex] = 0;
		}
	}
}