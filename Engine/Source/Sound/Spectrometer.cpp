/******************************************************************************

   !! Warning: 'Spectrometer' cannot be manually deleted/created as long as there's at least one Sound using it !!
      (not that clearing Spectrometer from Sound is not immediate, in case it's using it during the clear call)

/******************************************************************************/
#include "stdafx.h"
#define SUPPORT_FFTW    0 // fastest for all sizes, supports all       sizes, but needs a commercial license
#define SUPPORT_KISS    0 // fast only for pow2   , supports all       sizes, can be slow on non pow2 (especially prime numbers)
#define SUPPORT_FFTREAL 1 // faster than KISS     , supports only pow2 sizes

#include "../../../ThirdPartyLibs/begin.h"
   #if SUPPORT_FFTW
      #include "../../../ThirdPartyLibs/FFTW/lib/api/fftw3.h"
   #endif

   #if SUPPORT_KISS
      #undef USE_SIMD
      #include "../../../ThirdPartyLibs/KissFFT/kiss_fft.c"
      #include "../../../ThirdPartyLibs/KissFFT/kiss_fftr.c"
   #endif

   #if SUPPORT_FFTREAL
      #undef PI
      #undef SQRT2
      #include "../../../ThirdPartyLibs/FFTReal/ffft/FFTReal.h"
   #endif

#include "../../../ThirdPartyLibs/end.h"
namespace EE{
/******************************************************************************/
#if SUPPORT_FFTW
struct Complex
{
   fftw_complex c;
};

   // add these as members
   Memc<Dbl    > fftw_in;
   Memc<Complex> fftw_out;
#endif
/******************************************************************************/
Spectrometer::Spectrometer() {zero();}
Spectrometer::Spectrometer(Int resolution) : Spectrometer() {create(resolution);}

void Spectrometer::zero()
{
  _frequency=0;
  _window_length=0;
  _fft=null;
}
void Spectrometer::free()
{
#if SUPPORT_FFTW
   fftw_plan &fft=(fftw_plan&)_fft; if(fft){fftw_destroy_plan(fft); fft=null;}
#endif
#if SUPPORT_KISS
   kiss_fftr_cfg &fft=(kiss_fftr_cfg&)_fft; if(fft){::kiss_fft_free(fft); fft=null;}
#endif
#if SUPPORT_FFTREAL
   Delete((ffft::FFTReal<Flt>*&)_fft);
#endif
}
Spectrometer& Spectrometer::del()
{
   free();
  _image  .del();
  _samples.del();
  _fft_out.del();
   zero(); return T;
}
Spectrometer& Spectrometer::create(Int resolution)
{
   del();
  _window_length=1.0f/20; // 20 Hz
   Int buffer_length_ms=SOUND_TIME,
       window_length_ms=Floor(_window_length*1000); // use Floor so in worst case we allocate more rows (the smaller 'window_length_ms' the more 'rows')
   #if SUPPORT_KISS || SUPPORT_FFTREAL // Kiss and FFTReal round to NearestPow2 size, so we need to increase number of rows for worst case scenario
      // biggest scale of "NearestPow2(x)" compared to "x" is 1.333333.., tested using code: "Flt max=0; REP(10000000){Flt f=Flt(NearestPow2(i))/i; MAX(max, f);}"
      window_length_ms=window_length_ms*3/4; // we need to divide by 1.3333.. (4/3), which is the same as multiplying by 3/4
   #endif
   Int rows=(window_length_ms ? DivCeil(buffer_length_ms, window_length_ms) : 0); // we need enough rows to fit entire sound buffer
       rows+=2; // because we're doing interpolation, then we need more (1 for linear, 2 for cubic)
  _image.createSoftTry(resolution, rows, 1, IMAGE_F32); _image.clear();
   return T;
}
/******************************************************************************/
void Spectrometer::data(Ptr data, Int size, C SoundStream &stream, Long raw_pos)
{
   if(stream.frequency()!=_frequency)
   {
      free();
     _frequency=stream.frequency();
      Int samples=Round(_window_length*_frequency);

   #if SUPPORT_FFTW
      fftw_in .setNum(samples);
      fftw_out.setNum(samples/2+1);
     _fft=fftw_plan_dft_r2c_1d(fftw_in.elms(), fftw_in.data(), (fftw_complex*)fftw_out.data(), 0);
   #endif

   #if SUPPORT_KISS
      if(1)samples=NearestPow2(samples); // this is optional (may improve performance)
      else samples=      Ceil2(samples); // kiss_fftr requires even number of samples
     _fft=kiss_fftr_alloc(samples, 0, null, null);
     _fft_out.setNum((samples/2+1)*2); ASSERT(SIZE(kiss_fft_cpx)==SIZE(Flt)*2); // *2 because we need 'kiss_fft_cpx' instead of 'Flt' which is 2x bigger
   #endif

   #if SUPPORT_FFTREAL
      samples=NearestPow2(samples); // FFTReal requires pow2
     _fft=new ffft::FFTReal<Flt>(samples);
     _fft_out.setNum(samples);
   #endif

     _samples.setNumZero(samples); // zero in case 'sample_pos' jumps due to seeking
   }

   // checks to avoid div by zero
   if(! stream .block()
   || !_samples.elms ()
   || !_image  .is   ())return;

   Long sample=raw_pos/stream.block();
   Int  sample_pos=Unsigned(sample)%_samples.elms(); // expected number of samples in the '_samples' buffer, we need to calculate it every time instead of storing, in case seeking occurs, in which case the number of samples in the buffer must be in sync with 'raw_pos'

#if 0
   REPAO(_samples)=Cos(i/Flt(_samples.elms()-1)*Ms.pos().x*10000); sample_pos=_samples.elms();
#else
   for(Int data_samples=size/stream.block(); data_samples>0; )
   {
      Flt *dest=_samples.data()+sample_pos;
      Int  copy=Min(_samples.elms()-sample_pos, data_samples); data_samples-=copy;
      switch(stream.bytes())
      {
         case 1: // unsigned Byte
         {
            Byte* &src=(Byte*&)data;
            switch(stream.channels())
            {
               case 1:
               {
                  REP(copy)*dest++=(*src++ - 128)/128.0f;
                  sample_pos+=copy;
               }break;

               case 2:
               {
                  REP(copy)
                  {
                     Byte l=src[0], r=src[1]; src+=2;
                     *dest++=(l+r-256)/256.0f;
                  }
                  sample_pos+=copy;
               }break;
            }
         }break;

         case 2: // signed Short
         {
            Short* &src=(Short*&)data;
            switch(stream.channels())
            {
               case 1:
               {
                  REP(copy)*dest++=(*src++)/32768.0f;
                  sample_pos+=copy;
               }break;

               case 2:
               {
                  REP(copy)
                  {
                     Short l=src[0], r=src[1]; src+=2;
                     *dest++=(l+r)/65536.0f;
                  }
                  sample_pos+=copy;
               }break;
            }
         }break;

         case 3: // Int24
         {
            Int24* &src=(Int24*&)data;
            switch(stream.channels())
            {
               case 1:
               {
                  REP(copy)*dest++=(src++)->asInt()/Flt(-INT24_MIN);
                  sample_pos+=copy;
               }break;

               case 2:
               {
                  REP(copy)
                  {
                     *dest++=(src[0].asInt()+src[1].asInt())/Flt(-INT24_MIN*2); src+=2;
                  }
                  sample_pos+=copy;
               }break;
            }
         }break;
      }
      if(sample_pos>=_samples.elms())
      {
         sample_pos=0;
         if(1) // apply window (this is needed for more precise FFT calculation)
         {
            Flt mul=2.0f/(_samples.elms()-1);
            REPAO(_samples)*=BlendSmoothCube(i*mul-1);
         }

      #if SUPPORT_FFTW
         REPAO(fftw_in)=_samples[i]; fftw_execute((fftw_plan)_fft);
         REPA(fftw_out)
         {
            Flt frac=Flt(i+1)/fftw_out.elms(); // use this instead of "Flt frac=i/Flt(fftw_out.elms()-1);" because that would always set frac=0 at the start
            fftw_complex &complex=fftw_out[i].c;
            Flt value=SqrtFast(Sqr(complex[0])+Sqr(complex[1]));
            value*=frac;
          //value =Log2(value+1); instead of doing log here, do it only once for all collected samples below
            complex[0]=value;
         }
         Int end=fftw_out.elms(), y=Unsigned(sample/_samples.elms())%_image.h(); sample+=_samples.elms();
         REP(_image.w())
         {
            Int start=i*fftw_out.elms()/_image.w();
            Flt value=fftw_out[start].c[0]; for(Int i=start+1; i<end; i++)MAX(value, fftw_out[i].c[0]); // always take at least one value, in case start==end
            value=Log2(value+1);
          //value=BlendSmoothCube(i*0.2f - 26*Flt(y)/_image.h()); // can be used for testing
           _image.pixF(i, y)=value;
            end=start;
         }
      #endif

      #if SUPPORT_KISS
         kiss_fft_cpx *out=(kiss_fft_cpx*)_fft_out.data();
         kiss_fftr((kiss_fftr_cfg)_fft, _samples.data(), out);
         Int out_elms=_fft_out.elms()/2; // div by 2 because this is Flt but we're using it as 'kiss_fft_cpx'
         REP(out_elms)
         {
            Flt frac=Flt(i+1)/out_elms; // use this instead of "Flt frac=i/Flt(out_elms-1);" because that would always set frac=0 at the start
            kiss_fft_cpx &complex=out[i];
            Flt value=SqrtFast(Sqr(complex.r)+Sqr(complex.i));
            value*=frac;
          //value =Log2(value+1); instead of doing log here, do it only once for all collected samples below
            complex.r=value;
         }
         Int end=out_elms, y=Unsigned(sample/_samples.elms())%_image.h(); sample+=_samples.elms();
         REP(_image.w())
         {
            Int start=i*out_elms/_image.w();
            Flt value=out[start].r; for(Int i=start+1; i<end; i++)MAX(value, out[i].r); // always take at least one value, in case start==end
            value=Log2(value+1);
          //value=BlendSmoothCube(i*0.2f - 26*Flt(y)/_image.h()); // can be used for testing
           _image.pixF(i, y)=value;
            end=start;
         }
      #endif

      #if SUPPORT_FFTREAL
         ((ffft::FFTReal<Flt>*)_fft)->do_fft(_fft_out.data(), _samples.data());
         Int out_elms=_fft_out.elms()/2; // first half of the buffer is occupied by "real" and second half by "imaginary" (complex)
         REP(out_elms)
         {
            Flt frac=Flt(i+1)/out_elms; // use this instead of "Flt frac=i/Flt(out_elms-1);" because that would always set frac=0 at the start
            Flt &real=_fft_out[i];
            Flt value=SqrtFast(Sqr(real)+Sqr(_fft_out[i+out_elms]));
            value*=frac;
          //value =Log2(value+1); instead of doing log here, do it only once for all collected samples below
            real=value;
         }
         Int end=out_elms, y=Unsigned(sample/_samples.elms())%_image.h(); sample+=_samples.elms();
         REP(_image.w())
         {
            Int start=i*out_elms/_image.w();
            Flt value=_fft_out[start]; for(Int i=start+1; i<end; i++)MAX(value, _fft_out[i]); // always take at least one value, in case start==end
            value=Log2(value+1);
          //value=BlendSmoothCube(i*0.2f - 26*Flt(y)/_image.h()); // can be used for testing
           _image.pixF(i, y)=value;
            end=start;
         }
      #endif
      }
   }
#endif
}
/******************************************************************************/
void Spectrometer::get(Flt *meter, Int meter_elms, Flt time, FILTER_TYPE filter)
{
   if(meter && meter_elms>0)
   {
      if(Int s=_samples.elms())
      if(Int h=_image  .h   ())
      {
         Dbl sample=Dbl(time)*_frequency, frac=sample/s;
         Flt yf=Frac(frac, h);
         Int y;
         if(filter!=FILTER_NONE)yf-=0.5f; // need to offset by -0.5, because when we're at half row position (y=0.5) then we need to get exactly values of the first row, the same as y=0 with no interpolation, and in linear interpolation we will get it with y=0, this is because first row collects samples from start of the song, with window fading out at start and end, making most significant data coming from the middle of the window
         else                   y  =Unsigned(TruncL(yf))%h;
         if(meter_elms==resolution())switch(filter)
         {
            case FILTER_NONE  : CopyN(meter, &_image.pixF(0, y), meter_elms); break;
            case FILTER_LINEAR: REP(meter_elms)meter[i]=_image.pixelFLinear   (i, yf, false); break;
            default           : REP(meter_elms)meter[i]=_image.pixelFCubicFast(i, yf, false); break; // FILTER_CUBIC and others (actually use CubicFast because it uses fewer samples)
         }else
         {
            if(filter==FILTER_NONE)yf=y;
            Flt mul=Flt(resolution()-1)/(meter_elms-1); // this keeps edges
            switch(filter)
            {
               case FILTER_NONE  : // here for FILTER_NONE we're also using 'pixelFLinear' because we need to stretch horizontally
               case FILTER_LINEAR: REP(meter_elms)meter[i]=_image.pixelFLinear   (i*mul, yf, false); break;
               default           : REP(meter_elms)meter[i]=_image.pixelFCubicFast(i*mul, yf, false); break; // FILTER_CUBIC and others (actually use CubicFast because it uses fewer samples)
            }
         }
         return;
      }
      ZeroN(meter, meter_elms); // if not available then zero
   }
}
/******************************************************************************/
}
/******************************************************************************/
