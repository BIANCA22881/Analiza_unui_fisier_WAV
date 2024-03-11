//getctrldisplaybitmap();
//savebitmaptojpegfile();

#include <cvirte.h>		
#include <userint.h>
#include "simulare_si_achizitie_semnale.h"
#include <ansi_c.h>
#include <formatio.h>
#include <advanlys.h>
#include <utility.h>
#include <time.h>
#include "toolbox.h"

#define SAMPLE_RATE		0
#define NPOINTS			1

// Global variables
static int fs =44100;
int iStart=0;
/////////////////////
int waveInfo[2]; //waveInfo[0] = sampleRate
				 //waveInfo[1] = number of elements
double sampleRate = 0.0;
int npoints = 0;
double *waveData = 0;
static int panelHandle;
static int acqPanel ;
int interv=20;
int valRing;
double* filtruMediere,*filtruAlpha;
double* derivata;
int filtMed=16;
double alpha;
double skew,kurtosis;
char*fileName;
int bitmapID;
//anvelopa
double threshold=0;
ssize_t nrPeaks;
double*peakLocation;
double*peakAmplitudes;
double*peakSecondDerivates;
//pentru spectru
int N;
int StartSpect=0;
double* spectru;
double* autoSpectrum;
double* buffer;
double df;
double frequencyPeak,powerPeak;
WindowConst winConst;
double* downSampled;
//pentru spectru filtrat
WindowConst winConst2;
double* buffer2;
double* spectru2;
double* autoSpectrum2;
double df2;
double frequencyPeak2,powerPeak2;
int doSpectruF = 0;
double* dataFiltrata;
int tipFereastra, tipFiltru;


int main (int argc, char *argv[])
{
	int error = 0;
	
	/* initialize and load resources */
	nullChk (InitCVIRTE (0, argv, 0));
	errChk (panelHandle = LoadPanel (0, "simulare_si_achizitie_semnale.uir", PANEL));
	errChk (acqPanel = LoadPanel (0, "simulare_si_achizitie_semnale.uir", PANEL_2));
	
	/* display the panel and run the user interface */
	errChk (DisplayPanel (panelHandle));
	errChk (RunUserInterface ());

Error:
	/* clean up */
	if (panelHandle > 0)
		DiscardPanel (panelHandle);
	return 0;
}

int CVICALLBACK onExit (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
	}
	return 0;
}
int nrTreceriZero(double* vector,int n)
{
	int nrzero=0;
	for(int i=1;i<npoints;i++)
	{
		if(vector[i-1]*vector[i]<0)
			nrzero++;
	}
	return nrzero;
}
char* deblank(char* input)                                         
{
    int i,j;
    char *output=input;
    for (i = 0, j = 0; i<strlen(input); i++,j++)          
    {
        if (input[i]!=' ')                           
            output[j]=input[i];                     
        else
            j--;                                     
    }
    output[j]=0;
    return output;
}

int CVICALLBACK OnLoadButtonCB (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//executa script python pentru conversia unui fisierului .wav in .txt
			//LaunchExecutable("python main.py");
			
			//astept sa fie generate cele doua fisiere (modificati timpul daca este necesar
			//Delay(4);
			
			//incarc informatiile privind rata de esantionare si numarul de valori
			FileToArray ("c:\\Users\\student\\Desktop\\1309b-bvb\\cvibuild.proiect1\\Debug\\wafeInfo.txt", waveInfo, VAL_INTEGER, 2, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS,	 VAL_ASCII);
			sampleRate = waveInfo[SAMPLE_RATE];
			npoints = waveInfo[NPOINTS];
			
			//alocare memorie pentru numarul de puncte
			waveData = (double *) calloc(npoints, sizeof(double)); //valorile raw
			filtruMediere=(double *) malloc(npoints* sizeof(double)); //valorile pentru filtru mediere
			filtruAlpha=(double *) malloc(npoints* sizeof(double)); //valorile pentru filtru alpha
			derivata=(double *) malloc(npoints* sizeof(double)); //valorile pentru derivata
			dataFiltrata= (double *) malloc(npoints* sizeof(double)); //valorile pentru datele diltrate cu chebishev sau butterworth
			
			//pentru spectru simplu
			buffer= (double *) malloc(16384* sizeof(double));
			autoSpectrum= (double *) malloc(16384* sizeof(double));
			spectru =(double *) malloc(16384* sizeof(double));
			downSampled=(double *) malloc((npoints/4)* sizeof(double));
			//pentru ferestruire plus filtru
			buffer2= (double *) malloc(16384* sizeof(double));			
			autoSpectrum2= (double *) malloc(16384* sizeof(double));
			spectru2 =(double *) malloc(16384* sizeof(double));
			
			//incarcare din fisierul .txt in memorie (vector)
			FileToArray ("c:\\Users\\student\\Desktop\\1309b-bvb\\cvibuild.proiect1\\Debug\\waveData.txt", waveData, VAL_DOUBLE, npoints, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS,
						 VAL_ASCII);
			
			//afisare pe grapf
			PlotY (panel, PANEL_GRAPH, waveData, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			
			
			SetCtrlVal(panel, PANEL_NUMERIC, iStart/fs);
			SetCtrlVal(panel, PANEL_NUMERIC_2, (iStart+fs)/fs);
			
			
			/////////////////////////////////////////////////////////
			
			double min,max,medie,dispersie,mediana;
			int nrzero,imax,imin;
	
			MaxMin1D (waveData, npoints, &max, &imax, &min, &imin);
			Mean (waveData, npoints, &medie);
			Median (waveData, npoints, &mediana);
			StdDev (waveData, npoints, &medie, &dispersie);
			nrzero = nrTreceriZero(waveData,npoints);
			
			//histograma
			double* axisArray;
			axisArray=(double *) calloc(interv, sizeof(double));
			ssize_t *histogram;
			histogram=(ssize_t *) calloc(npoints, sizeof(ssize_t));
			Histogram(waveData, npoints, min-1, max+1, histogram, axisArray, interv);
			PlotXY (panel, PANEL_GRAPH_3, axisArray, histogram, interv, VAL_DOUBLE, VAL_INTEGER, VAL_VERTICAL_BAR, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_BLUE);
			
			SetCtrlVal (panel, PANEL_NUMERIC_3, min);
			SetCtrlVal (panel, PANEL_NUMERIC_4, max);
			SetCtrlVal (panel, PANEL_NUMERIC_9, imin);
			SetCtrlVal (panel, PANEL_NUMERIC_10, imax);
			SetCtrlVal (panel, PANEL_NUMERIC_8, medie);
			SetCtrlVal (panel, PANEL_NUMERIC_5, dispersie);
			SetCtrlVal (panel, PANEL_NUMERIC_6, mediana);
			SetCtrlVal (panel, PANEL_NUMERIC_7, nrzero);
			
			fileName=(char *) malloc(99999 * sizeof(char));
			
			sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal.jpg"); //generare nume fisier
			GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
			SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
			
			break;
	}
	return 0;
}

int CVICALLBACK onMainPanel (int panel, int event, void *callbackData,
							 int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitUserInterface (0);
			break;
	}
	return 0;
}

int CVICALLBACK onNext (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(iStart >= 0 && iStart <= fs*5-1) 
			{
				iStart+=fs;
				//valori raw
				DeleteGraphPlot (panel, PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
				PlotY (panel, PANEL_GRAPH, waveData+iStart, fs, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
				SetCtrlVal(panel, PANEL_NUMERIC, iStart/fs);
				SetCtrlVal(panel, PANEL_NUMERIC_2, (iStart+fs)/fs); 	
				
				//valori filtrate
				if(valRing==0)
				{
					DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
					PlotY (panel, PANEL_GRAPH_2, filtruMediere+iStart, fs, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 	 
				}
				else if(valRing==1)
				{
					DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
					PlotY (panel, PANEL_GRAPH_2, filtruAlpha+iStart, fs, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 
				}
				else
				{
					DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
					PlotY (panel, PANEL_GRAPH_2, derivata+iStart, fs, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 
				}
				
				switch (iStart/fs)
				{
					case 0:	
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec0-1.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec0-1.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec0-1.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec0-1.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 1:
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec1-2.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec1-2.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec1-2.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec1-2.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 2:
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec2-3.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec2-3.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec2-3.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec2-3.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 3:	
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec3-4.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec3-4.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec3-4.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec3-4.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 4:
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec4-5.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec4-5.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec4-5.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec4-5.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 5:
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec5-6.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec5-6.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec5-6.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec5-6.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
				}

			}
			break;
	}
	return 0;
}

int CVICALLBACK onPrev (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(iStart >= fs && iStart <= fs*6-1)
			{
				iStart-=fs;
				DeleteGraphPlot (panel, PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
				PlotY (panel, PANEL_GRAPH, waveData+iStart, fs, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
				SetCtrlVal(panel, PANEL_NUMERIC, iStart/fs);
				SetCtrlVal(panel, PANEL_NUMERIC_2, (iStart+fs)/fs);
				
				if(valRing==0)
				{
					DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
					PlotY (panel, PANEL_GRAPH_2, filtruMediere+iStart, fs, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 
				}
				else if(valRing==1)
				{
					DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
					PlotY (panel, PANEL_GRAPH_2, filtruAlpha+iStart, fs, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 	 
				}
				else
				{
					DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
					PlotY (panel, PANEL_GRAPH_2, derivata+iStart, fs, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 	 
				}
				switch (iStart/fs)
				{
					case 0:	
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec0-1.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec0-1.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec0-1.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec0-1.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 1:
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec1-2.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec1-2.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec1-2.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec1-2.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 2:
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec2-3.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec2-3.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec2-3.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec2-3.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 3:	
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec3-4.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec3-4.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec3-4.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec3-4.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 4:
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec4-5.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec4-5.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec4-5.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec4-5.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
					case 5:
						//valori raw
						sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Semnal raw\\Semnal_sec5-6.jpg"); //generare nume fisier
						GetCtrlDisplayBitmap(panel, PANEL_GRAPH, 1, &bitmapID);
						SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						
						//valori filtrate
						if(valRing==0)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere_sec5-6.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
						}
						else if(valRing==1)
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha_sec5-6.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						else
						{
							sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata_sec5-6.jpg"); //generare nume fisier
							GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
							SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);	
						}
						break;
				}
			}
			break;
	}
	return 0;
}

int CVICALLBACK onAplica (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panel, PANEL_RING, &valRing);
			if(valRing==0)
			{
				int i;
				//filtru de mediere
				//filtruMediere=(double *) calloc((npoints-filtMed+1), sizeof(double));
				double sum=0;
				for (i=0;i<filtMed;i++)
					sum+=waveData[i];
				filtruMediere[0]=sum/filtMed;
				for (i=1;i<npoints-filtMed;i++)
				{
					sum=sum - waveData[i-1]+waveData[i+filtMed-1];
					filtruMediere[i]=sum/filtMed;
				}
				for(int j=npoints-filtMed;j<npoints;j++)
					filtruMediere[j]=filtruMediere[i];
				DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
				PlotY (panel, PANEL_GRAPH_2, filtruMediere, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
				
				sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Mediere\\FiltruMediere.jpg"); //generare nume fisier
				GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
				SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
			}
			else if(valRing==1 )
			{
				//filtru cu element de ordinul I 
				//filtruAlpha=(double *) calloc((npoints-filtMed+1), sizeof(double));
				GetCtrlVal(panel,PANEL_RINGSLIDE,&alpha);
				filtruAlpha[0]=0;
				for(int i=2;i<npoints;i++)
				{
					filtruAlpha[i]=(1-alpha)*filtruAlpha[i-1]+alpha*waveData[i];
				}
				DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
				PlotY (panel, PANEL_GRAPH_2, filtruAlpha, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
				
				sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Filtru Alpha\\FiltruAlpha.jpg"); //generare nume fisier
				GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
				SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
			}
			else
			{	
				//derivata
				derivata[0]=waveData[0];
				for(int i=1;i<npoints;i++)
				{
					derivata[i]=waveData[i]-waveData[i-1];
				}
				DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
				PlotY (panel, PANEL_GRAPH_2, derivata, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 	 
				
				sprintf(fileName, "C:Users\\student\\Desktop\\Imagini_semnal\\Derivata\\Derivata.jpg"); //generare nume fisier
				GetCtrlDisplayBitmap(panel, PANEL_GRAPH_2, 1, &bitmapID);
				SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
			}
			break;
	}
	return 0;
}

int CVICALLBACK onReset (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			DeleteGraphPlot (panel, PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
			PlotY (panel, PANEL_GRAPH, waveData, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			//valori filtrate
			if(valRing==0)
			{
				DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
				PlotY (panel, PANEL_GRAPH_2, filtruMediere, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 	 
			}
			else if(valRing==1)
			{					
				DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
				PlotY (panel, PANEL_GRAPH_2, filtruAlpha, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 	 
			}
			else
			{
				DeleteGraphPlot (panel, PANEL_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
				PlotY (panel, PANEL_GRAPH_2, derivata, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED); 	 	 
			}
			break;
	}
	return 0;
}

int CVICALLBACK onKurtosis (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			Moment (waveData, 256, 4, &kurtosis);
			SetCtrlVal(panel,PANEL_NUMERIC_11,kurtosis);
			break;
	}
	return 0;
}

int CVICALLBACK onSkewness (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			Moment (waveData, 256, 3, &skew);
			SetCtrlVal(panel,PANEL_NUMERIC_12,skew);
			break;
	}
	return 0;
}

int CVICALLBACK onAnvelopa (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			PeakDetector (waveData, npoints, 0, 3, DETECT_PEAKS, ANALYSIS_TRUE, ANALYSIS_TRUE, &nrPeaks, &peakLocation, &peakAmplitudes, &peakSecondDerivates);
			int salt=50;
			int k=0;
			double*goodPeaksAmp=(double*)malloc(nrPeaks*sizeof(double));
			double*goodPeaksLoc=(double*)malloc(nrPeaks*sizeof(double));
			for(int i=0;i<nrPeaks;i+=salt)
			{
				int maxLocal=peakAmplitudes[i];
				int indMaxLocal=peakLocation[i];
				for(int j=i+1;j<i+salt;j++)
				{
					if(maxLocal<peakAmplitudes[j])
					{
						maxLocal=peakAmplitudes[j];
						indMaxLocal=peakLocation[j];
					}
					
				}
				goodPeaksAmp[k]=maxLocal;
				goodPeaksLoc[k]=indMaxLocal;
				k++;
			}
			PlotXY (panel, PANEL_GRAPH, goodPeaksLoc, goodPeaksAmp, k, VAL_DOUBLE, VAL_DOUBLE, VAL_CONNECTED_POINTS, VAL_SIMPLE_DOT,
					VAL_SOLID, 1, VAL_BLUE);
			PlotXY (panel, PANEL_GRAPH_4, goodPeaksLoc, goodPeaksAmp, k, VAL_DOUBLE, VAL_DOUBLE, VAL_CONNECTED_POINTS, VAL_SIMPLE_DOT,
					VAL_SOLID, 1, VAL_BLUE);
			
			break;
	}
	return 0;
}

int CVICALLBACK onSwitchPanels (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(panel == panelHandle)
			{
				SetCtrlVal(acqPanel, PANEL_2_BINARYSWITCH, 1);
				DisplayPanel(acqPanel);
				HidePanel(panel);
			}
			else
			{
				SetCtrlVal(panelHandle, PANEL_BINARYSWITCH, 0);
				DisplayPanel(panelHandle);
				HidePanel(panel);
			}
			break;
	}
	return 0;
}

int CVICALLBACK onFrequencyPanel (int panel, int event, void *callbackData,
								  int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitUserInterface (0);
			break;
	}
	return 0;
}

int CVICALLBACK onLoadSpectrum (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	int state;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(acqPanel,PANEL_2_TOGGLEBUTTON,&state);
			SetCtrlAttribute(acqPanel,PANEL_2_TIMER,ATTR_ENABLED,!state);
			break;
	}
	return 0;
}

int CVICALLBACK onTick (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	char unit[32]="V";
	switch (event)
	{
		case EVENT_TIMER_TICK:
			StartSpect++;
			GetCtrlVal(acqPanel,PANEL_2_RINGSLIDE,&N);
			if (StartSpect > 5)
				StartSpect=0;
			/*for(int i = 0;i<npoints;i+=4)
			{
				downSampled[i/4]=waveData[i]+waveData[i+1]+waveData[i+2]+waveData[i+3];
				downSampled[i/4]=downSampled[i/4]/4;
			}
			int frecEsantD=fs/4;
			int fNk=frecEsantD/2;
			int sec=StartSpect*frecEsantD;
			*/
			for(int i = 0;i<npoints;i+=8)
			{
				downSampled[i/8]=waveData[i]+waveData[i+1]+waveData[i+2]+waveData[i+3]+waveData[i+4]+waveData[i+5]+waveData[i+6]+waveData[i+7];
				downSampled[i/8]=downSampled[i/8]/8;
			}
			int frecEsantD=fs/8;
			int fNk=frecEsantD/2;
			int sec=StartSpect*frecEsantD;
			for(int i=0;i<16384;i++)
			{
				buffer[i]=0;
				autoSpectrum[i]=0;
				spectru[i]=0;
			}
			for(int i=sec;i<sec+N;i++)
			{
				buffer[i-sec]=downSampled[i];
			}
			//spectru
			ScaledWindowEx (buffer, N, RECTANGLE, 0, &winConst);
			AutoPowerSpectrum(buffer,N,1.0/frecEsantD,autoSpectrum,&df);
			PowerFrequencyEstimate(autoSpectrum,N/2,-1,winConst,df,7,&frequencyPeak,&powerPeak);
			SpectrumUnitConversion (autoSpectrum, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS,df, winConst, spectru, unit);
			
			SetCtrlVal(acqPanel,PANEL_2_NUMERIC,frequencyPeak);
			SetCtrlVal(acqPanel,PANEL_2_NUMERIC_2,powerPeak);
			
			//afisare spectru
			DeleteGraphPlot (acqPanel, PANEL_2_GRAPH, -1, VAL_IMMEDIATE_DRAW);
			PlotWaveform (acqPanel, PANEL_2_GRAPH, spectru, fNk, VAL_DOUBLE, 1.0, 0.0, 0.0, 1.0, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
			//afisare raw data pe al doilea panel
			DeleteGraphPlot (acqPanel, PANEL_2_GRAPH_2, -1, VAL_IMMEDIATE_DRAW);
			//PlotY (acqPanel, PANEL_2_GRAPH_2,downSampled ,npoints/4 , VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
			PlotY (acqPanel, PANEL_2_GRAPH_2,downSampled ,npoints/8 , VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
			
			////////////////////// daca se apasa pe buton se incepe spectrul si pentru valori filtrate //////////////////////
			
			GetCtrlVal(acqPanel,PANEL_2_TOGGLEBUTTON_2,&doSpectruF);
			
			///////////////////////////////////////

			
			if(doSpectruF == 0)
			{//atunci cand apas pe start
				GetCtrlVal(acqPanel,PANEL_2_RING,&tipFereastra);
				//0 flat top
				//1 welch
				GetCtrlVal(acqPanel,PANEL_2_RING_2,&tipFiltru);
				//0 butterworth trece sus pentru 1/3 din frecv inalte
				//1 chebysev ordin II de ordin 5 trece banda pentru 1/4 - 1/2 din spectru 
				
				if(tipFiltru == 0)
				{
					//	printf("\n-------1/2*fs =  %lf ",sampleRate*1/2);
					//	printf("\n-------1/3*frequencyPeak =  %lf ",frequencyPeak*1/3);
					
					Bw_HPF (downSampled, npoints/8,frecEsantD,fNk*2.0/3 , 5, dataFiltrata);
					//afisare data filtrata
					DeleteGraphPlot (acqPanel, PANEL_2_GRAPH_3, -1, VAL_IMMEDIATE_DRAW);
					//PlotY (acqPanel, PANEL_2_GRAPH_3,dataFiltrata ,npoints/4 , VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
					PlotY (acqPanel, PANEL_2_GRAPH_3,dataFiltrata ,npoints/8 , VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
					
					sec=StartSpect*frecEsantD;
					for(int i=0;i<16384;i++)
					{
						buffer2[i]=0;
						autoSpectrum2[i]=0;
						spectru2[i]=0;
					}
					for(int i=sec;i<sec+N;i++)
					{
						buffer2[i-sec]=dataFiltrata[i];
					}
					if(tipFereastra == 0)
					{
						ScaledWindowEx (buffer2, N, FLATTOP, 0, &winConst2);
						AutoPowerSpectrum(buffer2,N,1.0/frecEsantD,autoSpectrum2,&df2);
						PowerFrequencyEstimate(autoSpectrum2,N/2,-1,winConst2,df2,7,&frequencyPeak2,&powerPeak2);
						SpectrumUnitConversion (autoSpectrum2, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS,df, winConst2, spectru2, unit);
	
						SetCtrlVal(acqPanel,PANEL_2_NUMERIC_4,frequencyPeak2);
						SetCtrlVal(acqPanel,PANEL_2_NUMERIC_3,powerPeak2);
			
						
						DeleteGraphPlot (acqPanel, PANEL_2_GRAPH_4, -1, VAL_IMMEDIATE_DRAW);
						PlotWaveform (acqPanel, PANEL_2_GRAPH_4, spectru2, fNk, VAL_DOUBLE, 1.0, 0.0, 0.0, 1.0, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
						
						
					}
					else
					{
						ScaledWindowEx (buffer2, N, WELCH, 0, &winConst2);
						AutoPowerSpectrum(buffer2,N,1.0/frecEsantD,autoSpectrum2,&df2);
						PowerFrequencyEstimate(autoSpectrum2,N/2,-1,winConst2,df2,7,&frequencyPeak2,&powerPeak2);
						SpectrumUnitConversion (autoSpectrum2, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS,df, winConst2, spectru2, unit);
	
						SetCtrlVal(acqPanel,PANEL_2_NUMERIC_4,frequencyPeak2);
						SetCtrlVal(acqPanel,PANEL_2_NUMERIC_3,powerPeak2);
						
						DeleteGraphPlot (acqPanel, PANEL_2_GRAPH_4, -1, VAL_IMMEDIATE_DRAW);
						PlotWaveform (acqPanel, PANEL_2_GRAPH_4, spectru2, fNk, VAL_DOUBLE, 1.0, 0.0, 0.0, 1.0, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
						
					}
				}
				else
				{
					
				//	printf("\n-------1/2*fs =  %lf ",sampleRate*1/2);
				//	printf("\n-------1/4*frequencyPeak =  %lf ",frequencyPeak*1/4);
				//	printf("\n-------1/2*frequencyPeak =  %lf ",frequencyPeak*1/2);
					
					InvCh_BPF (downSampled, npoints/8, frecEsantD, 0.25*fNk, 0.5*fNk, 0.1, 5, dataFiltrata);
					//afisare data filtrata
					DeleteGraphPlot (acqPanel, PANEL_2_GRAPH_3, -1, VAL_IMMEDIATE_DRAW);
					//PlotY (acqPanel, PANEL_2_GRAPH_3,dataFiltrata ,npoints/4 , VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
					PlotY (acqPanel, PANEL_2_GRAPH_3,dataFiltrata ,npoints/8 , VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
					
					sec=StartSpect*(frecEsantD);
					for(int i=0;i<16384;i++)
					{
						buffer2[i]=0;
						autoSpectrum2[i]=0;
						spectru2[i]=0;
					}
					for(int i=sec;i<sec+N;i++)
					{
						buffer2[i-sec]=dataFiltrata[i];
					}
					if(tipFereastra == 0)
					{
						ScaledWindowEx (buffer2, N, FLATTOP, 0, &winConst2);
						AutoPowerSpectrum(buffer2,N,1.0/frecEsantD,autoSpectrum2,&df2);
						PowerFrequencyEstimate(autoSpectrum2,N/2,-1,winConst2,df2,7,&frequencyPeak2,&powerPeak2);
						SpectrumUnitConversion (autoSpectrum2, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS,df, winConst2, spectru2, unit);
	
						SetCtrlVal(acqPanel,PANEL_2_NUMERIC_4,frequencyPeak2);
						SetCtrlVal(acqPanel,PANEL_2_NUMERIC_3,powerPeak2);
						
						DeleteGraphPlot (acqPanel, PANEL_2_GRAPH_4, -1, VAL_IMMEDIATE_DRAW);
						PlotWaveform (acqPanel, PANEL_2_GRAPH_4, spectru2, fNk, VAL_DOUBLE, 1.0, 0.0, 0.0, 1.0, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
					
					}
					else
					{
						ScaledWindowEx (buffer2, N, WELCH, 0, &winConst2);
						AutoPowerSpectrum(buffer2,N,1.0/frecEsantD,autoSpectrum2,&df2);
						PowerFrequencyEstimate(autoSpectrum2,N/2,-1,winConst2,df2,7,&frequencyPeak2,&powerPeak2);
						SpectrumUnitConversion (autoSpectrum2, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS,df, winConst2, spectru2, unit);
	
						SetCtrlVal(acqPanel,PANEL_2_NUMERIC_4,frequencyPeak2);
						SetCtrlVal(acqPanel,PANEL_2_NUMERIC_3,powerPeak2);
						
						DeleteGraphPlot (acqPanel, PANEL_2_GRAPH_4, -1, VAL_IMMEDIATE_DRAW);
						PlotWaveform (acqPanel, PANEL_2_GRAPH_4, spectru2, fNk, VAL_DOUBLE, 1.0, 0.0, 0.0, 1.0, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
					}
				}
				
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK onStartSpectruFiltrat (int panel, int control, int event,
									   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK onTakePicture (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//valori raw
			struct tm*localTime;
			time_t currentTime;
			time(&currentTime);
			localTime=localtime(&currentTime);
			strcpy(fileName,"C:Users\\student\\Desktop\\Imagini_semnal\\Spectru\\valori raw");
			sprintf(fileName+strlen(fileName),"%04d-%02d-%02d_%02d_%02d",
				   localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
				   localTime->tm_hour,localTime->tm_min);
			strcat(fileName,".jpeg");
			GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH, 1, &bitmapID);
			SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
			DiscardBitmap(bitmapID);
			
			//spectru valori raw
			strcpy(fileName,"C:Users\\student\\Desktop\\Imagini_semnal\\Spectru\\spectru valori raw");
			sprintf(fileName+strlen(fileName),"%04d-%02d-%02d_%02d_%02d",
				   localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
				   localTime->tm_hour,localTime->tm_min);
			strcat(fileName,".jpeg");
			GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_2, 1, &bitmapID);
			SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
			DiscardBitmap(bitmapID);
			
			GetCtrlVal(acqPanel,PANEL_2_RING,&tipFereastra);
				//0 flat top
				//1 welch
			GetCtrlVal(acqPanel,PANEL_2_RING_2,&tipFiltru);
				//0 butterworth trece sus pentru 1/3 din frecv inalte
				//1 chebysev ordin II de ordin 5 trece banda pentru 1/4 - 1/2 din spectru 
			if(tipFiltru==0)
			{
				//valori filtrate bw
				strcpy(fileName,"C:Users\\student\\Desktop\\Imagini_semnal\\Spectru\\val_fil_bw");
				sprintf(fileName+strlen(fileName),"%04d-%02d-%02d_%02d_%02d",
					    localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
				    	localTime->tm_hour,localTime->tm_min);
				strcat(fileName,".jpeg");
				GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_3, 1, &bitmapID);
				SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
				DiscardBitmap(bitmapID);
				if(tipFereastra==0)
				{
				//spectru val filtrate bw cu ferestruire flattop
					strcpy(fileName,"C:Users\\student\\Desktop\\Imagini_semnal\\Spectru\\spectru_val_fil_bw_flattop");
					sprintf(fileName+strlen(fileName),"%04d-%02d-%02d_%02d_%02d",
	 			    		localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
	 					    localTime->tm_hour,localTime->tm_min);
					strcat(fileName,".jpeg");
					GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_4, 1, &bitmapID);
					SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
					DiscardBitmap(bitmapID);
			
				}
				else
				{
					//spectru valori filtrate bw cu ferestruire welch
					strcpy(fileName,"C:Users\\student\\Desktop\\Imagini_semnal\\Spectru\\spectru_val_fil_bw_welch");
					sprintf(fileName+strlen(fileName),"%04d-%02d-%02d_%02d_%02d",
	 			    		localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
	 					    localTime->tm_hour,localTime->tm_min);
					strcat(fileName,".jpeg");
					GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_4, 1, &bitmapID);
					SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
					DiscardBitmap(bitmapID);
				}
			}
			else
			{
				//val filtrate invCh
				strcpy(fileName,"C:Users\\student\\Desktop\\Imagini_semnal\\Spectru\\val_fil_ch2");
				sprintf(fileName+strlen(fileName),"%04d-%02d-%02d_%02d_%02d",
					    localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
						localTime->tm_hour,localTime->tm_min);
				strcat(fileName,".jpeg");
				GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_3, 1, &bitmapID);
				SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
				DiscardBitmap(bitmapID);
				if(tipFereastra==0)
				{
					//spectru val filtrate invCh cu ferestruire flattop
					strcpy(fileName,"C:Users\\student\\Desktop\\Imagini_semnal\\Spectru\\spectru_val_fil_ch2_flattop");
					sprintf(fileName+strlen(fileName),"%04d-%02d-%02d_%02d_%02d",
	 					    localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
	 					    localTime->tm_hour,localTime->tm_min);
					strcat(fileName,".jpeg");
					GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_4, 1, &bitmapID);
					SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
					DiscardBitmap(bitmapID);
				}
				else
				{
					//spectru val filtrate invCh cu ferestruire welch
					strcpy(fileName,"C:Users\\student\\Desktop\\Imagini_semnal\\Spectru\\spectru_val_fil_ch2_welch");
					sprintf(fileName+strlen(fileName),"%04d-%02d-%02d_%02d_%02d",
	 					    localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
	 					    localTime->tm_hour,localTime->tm_min);
					strcat(fileName,".jpeg");
					GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_4, 1, &bitmapID);
					SaveBitmapToJPEGFile(bitmapID, fileName, JPEG_PROGRESSIVE, 100);
					DiscardBitmap(bitmapID);
				}
			}
			
			break;
	}
	return 0;
}
