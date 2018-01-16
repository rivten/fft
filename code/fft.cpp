#include <rivten.h>
#include <rivten_math.h>

#define ZeroMemory(x) memset((x), sizeof(x), 0)
#define SEQUENCE_SIZE 1 << 12

u32 Length(char* Buffer)
{
	u32 Result = 0;
	char* C = Buffer;
	while(*C)
	{
		++C;
		++Result;
	}
	return(Result);
}

int main(int ArgumentCount, char** Arguments)
{
	float InSeq[SEQUENCE_SIZE];
	ZeroMemory(InSeq);
	float InPeriod = 100.0f;
	for(u32 Index = 0; Index < ArrayCount(InSeq); ++Index)
	{
		InSeq[Index] = Sin(2.0f * PI * (float(Index) / InPeriod)) + Sin(2.0f * PI * (float(Index) / 18.22f));
	}

	float Fraction = 2.0f * PI / float(SEQUENCE_SIZE);
	float OutSeq[SEQUENCE_SIZE];
	for(u32 k = 0; k < SEQUENCE_SIZE; ++k)
	{
		float Re = 0.0f;
		float Im = 0.0f;
		for(u32 n = 0; n < SEQUENCE_SIZE; ++n)
		{
			Re += InSeq[n] * Cos(Fraction * float(k) * float(n));
			Im -= InSeq[n] * Sin(Fraction * float(k) * float(n));

		}

		OutSeq[k] = SquareRoot(Re * Re + Im * Im);
	}

	FILE* FileHandle = fopen("output.txt", "w");
	Assert(FileHandle);
	char Buffer[512];
	for(u32 Index = 0; Index < SEQUENCE_SIZE; ++Index)
	{
		ZeroMemory(Buffer);
		sprintf(Buffer, "X_%i = %f\n", Index, OutSeq[Index]);
		fwrite(Buffer, Length(Buffer), 1, FileHandle);
	}
	fclose(FileHandle);

	return(0);
}
