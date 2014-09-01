// LZW.cpp : Defines the entry point for the console application.
//
// Author: Asad Ali
// Email: asad_82@yahoo.com

#include "stdafx.h"
#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DICTIONARY_LENGTH 4096
#define CODE_WORD_LENGTH 128
#define MIN_CHAR_INSERT 2

using  namespace std;

class LZWCompression
{
	
/*--------------------------Variables Declarations-----------------------------------*/
private:
	char  readFile[2048] , writeFile[2048];	// stores the filenames used for opening and writing new files

	unsigned char dictionary[DICTIONARY_LENGTH][CODE_WORD_LENGTH];
	int codeWordLength[DICTIONARY_LENGTH];
	int dictionaryIndex;
	int dictionaryHits;

	long totalBytes, bytesWritten;					// count the total number of bytes in file
	long double totalFileSize;
	int binary[12], binaryIndex;

	double estimatedBits;
	unsigned char byte, bit;
	int byteFillCount;

	struct Data
	{
		unsigned char dataByte;
		int dictHitIndex;
		bool partofDict;
		bool processed;
		bool dictHit;
		
	};

	Data *dataBuffer[CODE_WORD_LENGTH], *nodeBuffer[CODE_WORD_LENGTH], *node;
	int dataBufferIndex;

	//int remHitIndex[CODE_WORD_LENGTH];
	int processedIndex, writeIndex;
	int constructDictIndex;
	int recordMatchCount, remMaxMatchIndex;
	bool insertFlag, dictionaryEntry, dictHit, fetchNext;
	long countUncodedBytes, countCodedBytes;
	int freq[256];
	int charGIndex, maxMatchCount;
	unsigned char charToInsert[CODE_WORD_LENGTH+1];


public:
	FILE  *getPtr, *writePtr, *writePtr3,*writePtr2;/**writePtr1, *writePtr3;*/	// stores the file pointer after the file has been opened

/*--------------------------Initialize Variables -----------------------------------------*/

	LZWCompression()
	{
		*readFile = '\0'; 
		*writeFile = '\0';
		// clear the pointers
		getPtr = NULL; 
		writePtr = NULL;
		writePtr3 = NULL;

		dictionaryIndex = 256;
		// clear the dictionary
		for(int i=0;i<DICTIONARY_LENGTH;i++)
		{
			for (int j=0;j<CODE_WORD_LENGTH;j++)
			{
				dictionary[i][j]= '\0';
				//remHitIndex[j] = -1;
			}
			codeWordLength[i] = 0;
		}
		// add single characters
		unsigned char b=0;
		for (int j=0;j<256;j++)
		{
			dictionary[j][0] = b++;
			codeWordLength[j] = 1;
			freq[j] = 0;
		}
		totalBytes = 0;
		byte = bit = 0;
		estimatedBits = 0;
		byteFillCount = 0;
		dataBufferIndex = 0;
		processedIndex = writeIndex = 0;
		constructDictIndex = 0;
		recordMatchCount=0;
		remMaxMatchIndex=0;
		insertFlag = false;
		dictionaryEntry = false;
		dictHit = false;
		countUncodedBytes = 0; 
		countCodedBytes = 0;
		dictionaryHits = 0;
		charGIndex = maxMatchCount = 0;
		fetchNext = false;
		bytesWritten = 0;
	}// end of function


/*--------------------------Get File Names -----------------------------------------*/

	void GetFileNames()
	{
		cout<<"\n Enter File Name To Zip = ";
		cin>>readFile;
		cout<<"\n Enter Name Of Zipped File = ";
		cin>>writeFile;
	}// end of function
	
/*--------------------------Open File for Reading-----------------------------------------*/
	

	void OpenFiles()
	{
		if((getPtr=fopen(readFile,"rb"))==NULL)
		{  
			cout<<"\n Unable To Open File="<<readFile;
		    exit(1);          
		}


		if((writePtr=fopen(writeFile,"wb"))==NULL)
		{
			cout<<"\n Unable To Write File="<<writeFile;
			exit(1);
		}


		if((writePtr2=fopen("C:\\Github\\Encoded_File.txt","wb"))==NULL)
		{  
			cout<<"\n Unable To Open File="<<readFile;
		    exit(1);	        
		}

/*		if((writePtr2=fopen("C:\\CodingPattern.txt","wb"))==NULL)
		{  
			cout<<"\n Unable To Open File="<<readFile;
		    exit(1);	        
		}
*/

		if((writePtr3=fopen("C:\\Github\\Dictionary_Encode.txt","wb"))==NULL)
		{  
			cout<<"\n Unable To Open File="<<readFile;
		    exit(1);	        
		}

   }// end of function
	

/*--------------------------Read Data from File -----------------------------------------*/


	void ReadBytesFromFile()
	{
		unsigned char ch;

		while(!feof(getPtr))
		{   		   
		   ch=fgetc(getPtr);	// read one byte from file
		   node = new Data;
		   node->dataByte = ch;
		   node->partofDict = false;
		   node->processed = false;
		   node->dictHit = false;
		   node->dictHitIndex = -1;
		   dataBuffer[dataBufferIndex++] = node;

			if(dictionaryIndex == 2602)
				int a = 1;
			
			if(bytesWritten == 70459)
				int b = 1;

		   SearchDictionary();
			
		   if(dictionaryIndex < DICTIONARY_LENGTH)
		   {
				InsertInDictionary();
				fetchNext = false;
		   }
		   else // when the dictionary is full then only dictionary hits may occur
		   {
			   if(charGIndex == maxMatchCount && charGIndex < CODE_WORD_LENGTH)
			   {
				   fetchNext = true;
			   }
			   else if(fetchNext == true)
			   {
					int count = 0;
				    for(int m=constructDictIndex;m<(dataBufferIndex-1);m++)
					{
						if(dataBuffer[m]->dictHit == true)	
						{
							dataBuffer[m]->processed = true;
							count++;
						}
						//else
						//	cout<<"\n Error in loop";
					}
					constructDictIndex += count;
					fetchNext = false;

			   }// end of else if
			   else if(charGIndex > MIN_CHAR_INSERT)// && charGIndex < CODE_WORD_LENGTH)
			   {
					dataBuffer[constructDictIndex]->processed = true;
					dataBuffer[constructDictIndex]->partofDict = true;
					constructDictIndex++;
			   } // end of else if

		   }// end of if



		   WriteProcessedCharToDisk();

		   if (dataBufferIndex == CODE_WORD_LENGTH)
		   {
				if(constructDictIndex == 0)
				{					
					if(dataBuffer[constructDictIndex]->dictHit == true && charGIndex == CODE_WORD_LENGTH && dictionaryIndex < DICTIONARY_LENGTH)
					{
					   // flush the buffer and start again
						int count = 0;
						for(int m = 0; m < (dataBufferIndex - 1); m++)
						{
							if(dataBuffer[m]->dictHit == true)
							{
								dataBuffer[m]->processed = true;
								count++;
							}
							else
								cout<<"\n Error In loop";
						}// end of for
						constructDictIndex += count;
						recordMatchCount = 0;
						insertFlag = false;
					} 
					else 
					{
						dataBuffer[constructDictIndex]->processed = true;
						dataBuffer[constructDictIndex]->partofDict = true;
						constructDictIndex++;
					} // end of if

					// write processed data to disk
					WriteProcessedCharToDisk();
				}// end of if
			    int temp=0;
				for(int y=constructDictIndex; y<dataBufferIndex; y++)
				{
					dataBuffer[temp] = NULL;
					dataBuffer[temp++] = dataBuffer[y];
				}// end of for
				constructDictIndex = 0;				
				if(temp >= CODE_WORD_LENGTH)
					cout<<"\n Error in dataBuffer Length";
				dataBufferIndex = temp;
				writeIndex = constructDictIndex;

		   }// end of if

		  
		   totalBytes++;		// count total bytes in file
	
		}// end of while



		if(constructDictIndex != dataBufferIndex)
		{			
			for(int y=constructDictIndex; y<dataBufferIndex; y++)
			{
				dataBuffer[y]->processed = true;
				if(dataBuffer[y]->dictHit == false) // if dict hit has not occured previously
					dataBuffer[y]->partofDict = true;
			} // end of for
			constructDictIndex = dataBufferIndex;

			// write processed data to disk
			WriteProcessedCharToDisk();						
			//cout<<"\n Error: Buffer not flushed";
		} // end of if

		// if the byte is not full then fill it for writing 
		if(byteFillCount != 0 && byteFillCount < 8)
		{  
			REPEAT:
				
				if(byteFillCount < 8)
				{ 
					bit = 0;	// tells that it is uncoded character
					byte = byte | bit;										
					byteFillCount++;
					if(byteFillCount < 8)
						byte<<=1;
					goto REPEAT;
				} 
			
			fputc(byte,writePtr);
		} // end of if



		cout<<"\n Dictionary Length = "<<(dictionaryIndex);
		cout<<"\n Dictionary Hits = "<<dictionaryHits;
		cout<<"\n Total Bytes in Original File = "<<totalBytes;
		cout<<"\n Estimated Size of New File (bytes)= "<<(estimatedBits/8);
		cout<<"\n Uncoded Bytes = "<<countUncodedBytes;
		cout<<"\n Coded Bytes = "<<countCodedBytes;
		cout<<"\n Fine 1.";

		// for information and debugging purpose
		int uniqueSymbols = 0;
		for(int i=0;i<256;i++)
		{
			if(freq[i]!=0)
			{
				cout<<"\n "<<i<<" ------- "<<freq[i];
				uniqueSymbols++;
			}// end of if
		}// end of for

		cout<<"\n Unique Symbols = "<<uniqueSymbols;



	} // end of function

/*--------------------------Search Dictionary-----------------------------------------*/

	void SearchDictionary()
	{
		
		int matchCount[DICTIONARY_LENGTH];
		
		// find / group the characters that need to be inserted
		charGIndex = 0;	
		for(int k=constructDictIndex;k<dataBufferIndex;k++)
		{
			nodeBuffer[charGIndex] = dataBuffer[k];
			charToInsert[charGIndex++] = nodeBuffer[charGIndex]->dataByte;
		}		   
		charToInsert[charGIndex] ='\0';

		// hashing can later be used here instead of linear searching
		maxMatchCount = 0;
		dictHit = false;

		for(int j=0;j<(dictionaryIndex);j++)
		{
			if(charToInsert[0]==dictionary[j][0]) // check other characters for a match if the first character matches
			{
				matchCount[j] = 0;
				for(int k=0;k<codeWordLength[j];k++)				
				{
					if(charToInsert[k]==dictionary[j][k])
					{
						matchCount[j]++;
					}
					else
					{
						matchCount[j] = 0;
						break;
					}
				}// end of for

				
				if (matchCount[j] > maxMatchCount)
				{
					maxMatchCount = matchCount[j];
					remMaxMatchIndex = j;
				}

				// mean the dictionary entry already exists for this character sequence
				if(maxMatchCount == charGIndex)
				{
					if(charGIndex > MIN_CHAR_INSERT && j > 255)
					{	
						bool justOnce = false;
						for(int m=constructDictIndex;m<dataBufferIndex;m++)
						{
							if(dataBuffer[m]->processed == false && dataBuffer[m]->partofDict == false)
							{
								//dataBuffer[m]->partofDict = false;
								dataBuffer[m]->dictHit = true;
								if(justOnce == false)
								{
									dataBuffer[m]->dictHitIndex = j;
									justOnce = true;
									// for debugging only
									if(j == 307)
										int a = 1;
								}
							}
							else if(dataBuffer[m]->partofDict == true)
							{
								maxMatchCount = charGIndex = 0;
								dataBuffer[m]->processed = true;
								constructDictIndex++;
								break;
							}
						}// end of for

						if(justOnce == true)
						{
							dictHit = true;					// set the global hit flag
							dictionaryHits++;				// increment the counter
							recordMatchCount = charGIndex;	// record the number of characters found
							insertFlag = true;		
						}
						//fprintf(writePtr1,"\nDict Hit Index = %d",j);
					}// end of if

					break;
				} // end of if

				
			} // end of if
				   
		} // end of for


	}// end of function


/*--------------------------Insert Into Dictionary-----------------------------------------*/

	void InsertInDictionary()
	{

		if(dictionaryIndex == 1359)
			int a = 1;

		char firstMatch;
		int localCount = 0;

		if (dictionaryIndex < DICTIONARY_LENGTH)
		{
			if(maxMatchCount != charGIndex && charGIndex > MIN_CHAR_INSERT && charGIndex < CODE_WORD_LENGTH) // we want to add only those character sequences to the dictionary that are atleast 3 characters long
			{
				// insert into dictionary as the character sequence is new and save the node state				
				fprintf(writePtr3,"\nIDE = %d  --->",dictionaryIndex);
				for(int k = 0, m = constructDictIndex; k<charGIndex;k++, m++)
				{
					if(k == 0)
						firstMatch = charToInsert[k];
					else
					{
						if(firstMatch==charToInsert[k])
						{
							localCount++;
						}
					}// end of else
					dictionary[dictionaryIndex][k] = charToInsert[k];
					codeWordLength[dictionaryIndex] = k+1;
					
					if(nodeBuffer[k]->dictHit == false)
						nodeBuffer[k]->partofDict = true;
					
					dataBuffer[m] = nodeBuffer[k];
					fputc(charToInsert[k],writePtr3);
				}// end of for

				dictionaryIndex++;
				
				nodeBuffer[0]->processed = true;
				dataBuffer[constructDictIndex] = nodeBuffer[0];
				constructDictIndex++;
				
				if(insertFlag == true || localCount == (charGIndex-1))
				{
					if(recordMatchCount == 0)
						recordMatchCount = (localCount + 1);

					for(int h=constructDictIndex; h<(constructDictIndex + (recordMatchCount - 1)) ; h++)
					{
						dataBuffer[h]->processed = true;
						if(localCount == (charGIndex-1) && dataBuffer[h]->dictHit == false)
						{
							dataBuffer[h]->partofDict = true;
						}
					}

					//if(insertFlag == true)
						constructDictIndex+=(recordMatchCount-1);
					//else
					//	constructDictIndex+= (localCount);
					insertFlag = false;
					recordMatchCount = 0;
				}// end of if

				

				//dictionaryEntry = true;

			}// end of if
			   
		} // end of if   
	
	}// end of function	

	
/*--------------------------Write Processed Data to Disk-----------------------------------------*/


	void WriteProcessedCharToDisk()
	{

		bool writingSuccess = false;
		// write the charaters / codes to file that have been processed
		for(int w=writeIndex;w<constructDictIndex;w++)
		{
			// if part of dictionary or uncoded
			if(dataBuffer[w]->processed == true && dataBuffer[w]->partofDict == true) 
			{					
				// fill the byte, but first convert it into binary					

				DecToBin((int)dataBuffer[w]->dataByte);
				
				freq[(int)dataBuffer[w]->dataByte]++;
				// for debugging
				fputc(dataBuffer[w]->dataByte, writePtr2);

				bit = 0;	// tells that it is uncoded character
				byte = byte | bit;					
				byteFillCount++;
				if(byteFillCount < 8)
					byte <<= 1;

				if(byteFillCount == 8)
				{
					fputc(byte,writePtr);
					//fputc('-',writePtr2);
					freq[(int)byte]++;
					byte = 0;
					byteFillCount = 0;
					bytesWritten++;
				} // end of if

				for(int z=0;z<8;z++)
				{
					bit = binary[z];
					byte = byte | bit;
					byteFillCount++;
					if(byteFillCount < 8)
						byte <<= 1;

					if(byteFillCount == 8)
					{
						fputc(byte,writePtr);
						//fputc('-',writePtr2);
						byte = 0;
						byteFillCount = 0;
						bytesWritten++;
					} // end of if

				}// end of for

				//cout<<fileBuffer[w];
				estimatedBits+=9;
				countUncodedBytes++;
				writingSuccess = true;
			}
			else // if it is a dictionary hit
			{
				if(dataBuffer[w]->processed == true && dataBuffer[w]->dictHit == true && dataBuffer[w]->dictHitIndex != -1)
				{
					// for debugging					
					if(dataBuffer[w]->dictHitIndex < 256)
						cout<<"\n Error in Writing = "<<dataBuffer[w]->dictHitIndex;

					// fill the byte, but first convert it into binary
					DecToBin((int)dataBuffer[w]->dictHitIndex);
					
					fprintf(writePtr2,"< %d -- ",dataBuffer[w]->dictHitIndex);
					for(int u=0;u<codeWordLength[dataBuffer[w]->dictHitIndex];u++)
					{
						fputc(dictionary[dataBuffer[w]->dictHitIndex][u],writePtr2);
					}
					fputc('>',writePtr2);

					bit = 1;	// tells that it is coded character
					byte = byte | bit;
					byteFillCount++;
					if(byteFillCount < 8)
						byte <<= 1;

					if(byteFillCount == 8)
					{
						fputc(byte,writePtr);
						//fputc('*',writePtr2);
						byte = 0;
						byteFillCount = 0;
						bytesWritten++;
					} // end of if

					for(int z=0;z<12;z++)
					{
						bit = binary[z];
						byte = byte | bit;
						byteFillCount++;
						if(byteFillCount < 8)
							byte <<= 1;

						if(byteFillCount == 8)
						{
							fputc(byte,writePtr);
							//fputc('*',writePtr2);
							byte = 0;
							byteFillCount = 0;
							bytesWritten++;
						} // end of if

					}// end of for

					writingSuccess = true;
					estimatedBits+=13;
				} // end of if
				countCodedBytes++;

			}
		} // end of for

		if(writingSuccess == false && writeIndex!=constructDictIndex)
			cout<<"\n Error in writing to disk";

		writeIndex = constructDictIndex;




	} // end of function






/*
		//cout<<"\n Output for file \n";
		//for(int k=0;k<outputIndex;k++)
		//printf("%s",outputBuffer);

*/


/*--------------------------Decimal to Binary Converion-----------------------------------------*/
	void DecToBin(int data)
	{
		int rem = 0;
		data = abs(data);

		binaryIndex = 0;
		for(int g=0;g<12;g++)
			binary[g]=0;
		
		while(data > 0)// || data!=1)
		{
			rem = data % 2;
			if(rem == 0)
				binary[binaryIndex++] = 0;
			else
				binary[binaryIndex++] = 1;

			data = data / 2;
		}


	}// end of function


/*--------------------------Get File Size-----------------------------------------*/

	void GetFileSize()
	{
		fpos_t filepos;
		fseek(getPtr,0,SEEK_SET);
		fgetpos(getPtr,&filepos);
		fseek(getPtr,0,SEEK_END);		// seeks to the end of file
		totalFileSize = ftell(getPtr);   // tells the size of the file
		cout<<"\nTotal File Size = "<<totalFileSize;
	}// end of function





/*--------------------------Close File Pointers-----------------------------------------*/

	void closepointers()
	{
		fclose(getPtr);
		fclose(writePtr);
		fclose(writePtr3);

		//fclose(writePtr1);
		//fclose(writePtr2);
		cout<<"\n Fine 6.";
	}// end of function



};   // end of class


void main()
{
	LZWCompression compressFile;
	compressFile.GetFileNames();
	compressFile.OpenFiles();
	compressFile.ReadBytesFromFile();	
	compressFile.closepointers();

	cout<<"Done";
	//getch();

}
