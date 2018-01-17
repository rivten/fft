#include <rivten.h>
#include <rivten_math.h>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#define ZeroMemory(x) memset((x), sizeof(x), 0)
#define SEQUENCE_SIZE_LOG_2 15
#define SEQUENCE_SIZE 1 << SEQUENCE_SIZE_LOG_2

global_variable bool GlobalRunning = true;

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

float SignalInput(float t)
{
	float InPeriod = 100.0f;
	float Result = Sin(2.0f * PI * (t / InPeriod)) + 0.5f * Sin(2.0f * PI * (t / 18.22f));

	return(Result);
}

u32 BitReverse(u32 B, u32 Log2)
{
	u32 Result = 0;
	for(u32 Index = 0; Index < Log2; ++Index)
	{
		u32 Bit = (B >> Index) & 1;
		Result = Result | (Bit << (Log2 - Index - 1));
	}
	return(Result);
}

struct complex_number
{
	float Re;
	float Im;
};

complex_number
ExponentialI(float X)
{
	complex_number Result = {};
	Result.Re = Cos(X);
	Result.Im = Sin(X);
	
	return(Result);
}

complex_number
Multiply(complex_number A, complex_number B)
{
	complex_number Result = {};
	Result.Re = A.Re * B.Re - A.Im * B.Im;
	Result.Im = A.Re * B.Im + A.Im * B.Re;

	return(Result);
}

complex_number
Add(complex_number A, complex_number B)
{
	complex_number Result = {};
	Result.Re = A.Re + B.Re;
	Result.Im = A.Im + B.Im;

	return(Result);
}

complex_number
Substract(complex_number A, complex_number B)
{
	complex_number Result = {};
	Result.Re = A.Re - B.Re;
	Result.Im = A.Im - B.Im;

	return(Result);
}

inline float
Modulus(complex_number C)
{
	float Result = SquareRoot(C.Re * C.Re + C.Im * C.Im);
	return(Result);
}

void
FFT(float* Input, float* Output, u32 SampleCountLog2)
{
	// NOTE(hugo) : Algorithm taken from the
	// Wikipedia algorithm : Cooley-Tukey FFT algorithm
	Assert(SampleCountLog2 < 32);
	u32 SampleCount = 1 << SampleCountLog2;

	complex_number* X = AllocateArray(complex_number, SampleCount);

	for(u32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
	{
		// TODO(hugo): Cache-friendly ?
		u32 FourierSampleIndex = BitReverse(SampleIndex, SampleCountLog2);
		Assert(FourierSampleIndex < SampleCount);
		X[FourierSampleIndex] = {Input[SampleIndex], 0.0f};
	}

	for(u32 s = 0; s < SampleCountLog2; ++s)
	{
		u32 m = 1 << s;
		complex_number Wm = ExponentialI(- 2.0f * PI / float(m));
		for(u32 k = 0; k < SampleCount; k += m)
		{
			complex_number W = {1.0f, 0.0f};
			for(u32 j = 0; j < (m / 2); ++j)
			{
				complex_number t = Multiply(W, X[k + j + m / 2]);
				complex_number u = X[k + j];
				X[k + j] = Add(u, t);
				X[k + j + m / 2] = Substract(u, t);
				W = Multiply(W, Wm);
			}
		}
	}

	for(u32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
	{
		Output[SampleIndex] = Modulus(X[SampleIndex]);
	}

	Free(X);
}

int main(int ArgumentCount, char** Arguments)
{
	float InSeq[SEQUENCE_SIZE];
	ZeroMemory(InSeq);

	float t0 = 0.0f;
	float SamplingPeriod = 0.1f;

	for(u32 Index = 0; Index < ArrayCount(InSeq); ++Index)
	{
		InSeq[Index] = SignalInput(t0 + Index * SamplingPeriod);
	}

#if 0
	float Fraction = 2.0f * PI / float(SEQUENCE_SIZE);
	float OutSeq[SEQUENCE_SIZE >> 1];
	for(u32 k = 0; k < ArrayCount(OutSeq); ++k)
	{
		float Re = 0.0f;
		float Im = 0.0f;
		for(u32 n = 0; n < ArrayCount(InSeq); ++n)
		{
			Re += InSeq[n] * Cos(Fraction * float(k) * float(n));
			Im -= InSeq[n] * Sin(Fraction * float(k) * float(n));

		}

		OutSeq[k] = SquareRoot(Re * Re + Im * Im);
	}
#else
	float OutSeq[SEQUENCE_SIZE];
	FFT(InSeq, OutSeq, SEQUENCE_SIZE_LOG_2);
#endif

#if 0
	FILE* FileHandle = fopen("output.txt", "w");
	Assert(FileHandle);
	char Buffer[512];
	for(u32 Index = 0; Index < ArrayCount(OutSeq); ++Index)
	{
		ZeroMemory(Buffer);
		sprintf(Buffer, "X_%i = %f  (T = %f / f = %f)\n", Index, OutSeq[Index],
				SamplingPeriod * float(SEQUENCE_SIZE) / float(Index), float(Index) / (float(SEQUENCE_SIZE) * SamplingPeriod));
		fwrite(Buffer, Length(Buffer), 1, FileHandle);
	}
	fclose(FileHandle);
#endif
	float MaxSampleValue = 0.0f;
	for(u32 SampleIndex = 0; SampleIndex < SEQUENCE_SIZE; ++SampleIndex)
	{
		float SampleValue = OutSeq[SampleIndex];
		if(SampleValue > MaxSampleValue)
		{
			MaxSampleValue = SampleValue;
		}
	}

	// NOTE(hugo): SDL Fourier Transform Display
	s32 SDLInitResult = SDL_Init(SDL_INIT_VIDEO);
	Assert(SDLInitResult == 0);
	SDL_Window* Window = SDL_CreateWindow("fft", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			960, 540, 
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
	Assert(Window);
	SDL_ShowWindow(Window);

	SDL_Renderer* Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
	Assert(Renderer);

	while(GlobalRunning)
	{
		// NOTE(hugo): Input processing
		// {
		SDL_Event Event = {};
		for(;;)
		{
			s32 PendingEvents = 0;
			PendingEvents = SDL_PollEvent(&Event);
			if(!PendingEvents)
			{
				break;
			}

			switch(Event.type)
			{
				case SDL_QUIT:
				{
					GlobalRunning = false;
				} break;
			}
		}
		// }

		SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
		SDL_RenderClear(Renderer);
		SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);

		// NOTE(hugo): Finding sample rectangle size
		s32 WindowWidth = 0;
		s32 WindowHeight = 0;
		SDL_GetWindowSize(Window, &WindowWidth, &WindowHeight);
		//u32 SampleRectWidth = Ceil(float(WindowWidth) / float(SEQUENCE_SIZE / 2));
		Assert(SEQUENCE_SIZE >= 2 * WindowWidth);
		u32 SamplesPerWidthPixel = Floor(float(SEQUENCE_SIZE) / (2.0f * float(WindowWidth)));

		// NOTE(hugo): We do not take the whole screen for the max value
		// Only 90% of it
		float SampleRectHeightPerUnit = (0.90f * float(WindowHeight) / MaxSampleValue);

		for(u32 PixelIndex = 0; PixelIndex < u32(WindowWidth); ++PixelIndex)
		{
			// TODO(hugo): For now we select the maximum value on a 
			// particular pixel. Improve this somehow !
			float MaxSampleValueInPixel = 0.0f;
			for(u32 SampleIndex = 0; SampleIndex < SamplesPerWidthPixel; ++SampleIndex)
			{
				float SampleValue = OutSeq[SampleIndex + SamplesPerWidthPixel * PixelIndex];
				if(SampleValue > MaxSampleValueInPixel)
				{
					MaxSampleValueInPixel = SampleValue;
				}
			}
			SDL_Rect SampleRect = {};
			SampleRect.x = PixelIndex;
			SampleRect.h = u32(MaxSampleValueInPixel * SampleRectHeightPerUnit);
			SampleRect.y = WindowHeight - SampleRect.h;
			SampleRect.w = 1;
			SDL_RenderFillRect(Renderer, &SampleRect);
		}

		SDL_RenderPresent(Renderer);
	}

    SDL_Quit();

	return(0);
}
