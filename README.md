# P-B Footprint Reversal System Study

Trading the stock market requires that you have an edge over your competition; and in my experience, being able to understand and read order flow gives traders a HUGE edge. The biggest drawback with order flow trading is the heavy strain and stress induced from deciphering the constant flow of the market. To reduce that stress and add some HP back to your life, I created an advanced custom study that uses 4 condition triggers to help validate your trades, improve entries and exits, and to spot major reversals.



## About the Study
The trading system credit goes to Adam from [JumpstartTrading](https://www.jumpstarttrading.com/) and is outlined in his blog post [The Ultimate Guide To Profiting From Footprint Charts](https://www.jumpstarttrading.com/footprint-chart/). It is a very powerful method of evaluation and all I am doing is automating the process. If you want a more detailed explanation of using footprint charts I implore you to read his blog post. Below I will just go over the evaluation method I used in my study.

### Method of Evaluation
1. Compare the relative volume of the current bar with the previous. We want the volume to be increasing.
2. Check for Unfinished Auctions at the High and Low of the bars.
3. Check for Buy and Sell diagonal imbalances.
4. Check for p/b volume profiles by using bar POC and comparing it with a user specified bar limit.

![Image of Study](https://github.com/FutTrader/footprint-system/blob/assets/FootprintChartWithPBReversal.PNG)

## Getting Started

These instructions will get you a copy of the custom study up and running on your local machine.

### Prerequisites

Platform
```
Sierra Chart Package 5
```

### Installing

Manually install Advanced Custom Study files on Sierra Chart using [SC forum instructions](https://www.sierrachart.com/index.php?page=doc/UsingAdvancedCustomStudies.php#ManuallyInstallingAdvancedCustomStudyRelatedFiles).

Download the DLL file and save it in your SierraChart Data folder

```
e.g. C:\SierraChart\Data
```

Add the study to your Footprint Chart

```
Listed under: FootprintReversalSystem (FootprintReversalSystem_64.dllv1988)
```
## Study Settings

In order for the study to work properly, you will need to add the Numbers Bars Calculated Study to your Footprint Chart as well. This study has only been evaluated using **Range Bars** for my Footprint Charts. 

#### LN1 & 2: Alert Signals
Enable or disable alerts for buy and sell signals

#### LN3: Reversal Alert Number
Choose your audible alert number.

#### LN4: (CRITICAL) Input NumberBars2
After adding the NumberBarsCalculated study into your chart, select the proper study ID from the dropdown menu.

#### LN5: (CRITICAL) Relative Volume Study (SG14)
Select SG14 from the dropdown menu. This will provide access to the VolBarDiff array from the NumberBarsCalculated study.

#### LN6: Draw Style Mode to Top or Bottom of Bar
Once an alert is triggered, a transparent rectange box will fill in that column. Select if you want it to engulf the bar or stop at the bottom of the bar.

#### LN7: Buy/Sell Imbalance Percentage Threshold
This value is used in the comparison of the diagonal Buy Volume with the Ask Volume.

#### LN8: Enable Zero Bid/Ask Compares
Set to No if you don't want to evaluate Bid or Ask diagonal with a 0 print.

#### LN9: Zero Value Compare Action
Choose how you want to evaluate if you want to compare with a 0 print value.
Set 0 to 1 - Will take the actual value of Bid or Ask and multiply by 100
Set to +/- 1000% - Will set the Volume Ratio to 1000.

#### LN10: Number of TICKS from High/Low for PB Reversal
This study assumes that you are using a static range bar chart. Depending on how large your range bar chart is, select a value < **MAX_TICK** to choose a limit for your volume point of control evaluation. The denomination is in ticks. A smaller value will give you a looser evaluation of the p/b signals while a tighter one will give less triggers.

## Author

* **Patrick Senas** - *Initial work* - [FutTrader](https://github.com/FutTrader)

## License

This project is licensed under the MIT License - see the [LICENSE.md](https://github.com/FutTrader/footprint-system/blob/master/RISKDISCLAIMER.md) file for details

## Acknowledgments

* Adam from [JumpstartTrading](https://www.jumpstarttrading.com/)

## Disclaimer

Refer to [Risk Disclaimer](https://github.com/FutTrader/footprint-system/blob/master/RISKDISCLAIMER.md)
