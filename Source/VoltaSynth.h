#pragma once
#include <JuceHeader.h>
#include <cmath>

struct VoltaSound : public juce::SynthesiserSound {
 bool appliesToNote(int) override{return true;} bool appliesToChannel(int) override{return true;}
};

class VoltaVoice : public juce::SynthesiserVoice {
public:
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<VoltaSound*>(s)!=nullptr;}
 void prepare(double sr,int block){sampleRate=sr; env.setSampleRate(sr); filter.reset(); juce::dsp::ProcessSpec sp{sr,(juce::uint32)block,1}; filter.prepare(sp);}
 void startNote(int note,float velocity,juce::SynthesiserSound*,int) override{
   hz=(float)juce::MidiMessage::getMidiNoteInHertz(note); level=velocity; phase1=phase2=subPhase=0; env.noteOn();
 }
 void stopNote(float,bool tail) override{if(tail)env.noteOff();else{env.reset();clearCurrentNote();}}
 void pitchWheelMoved(int v) override{bend=std::pow(2.0f,((v-8192)/8192.0f)*2.0f/12.0f);}
 void controllerMoved(int,int) override{}
 void setParams(float mix,float det,float cutoff,float res,float a,float d,float s,float r,float tube,float iron,float voltage){
   oscMix=mix;detune=det;cut=cutoff;reso=res;tubeDrive=tube;ironAmt=iron;voltageAmt=voltage;
   env.setParameters({a,d,s,r});
 }
 void renderNextBlock(juce::AudioBuffer<float>& out,int start,int n) override{
   if(!isVoiceActive())return;
   float drift=1.0f+voltageAmt*0.003f*std::sin(driftPhase);
   float f1=hz*bend*drift, f2=hz*bend*std::pow(2.0f,detune/1200.0f)/drift;
   filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
   filter.setCutoffFrequency(juce::jlimit(20.0f,18000.0f,cut*(1.0f+voltageAmt*0.08f*std::sin(driftPhase*0.37f))));
   filter.setResonance(juce::jlimit(0.1f,1.35f,reso));
   for(int i=0;i<n;++i){
     float s1=saw(phase1),s2=saw(phase2),sub=0.22f*(subPhase<juce::MathConstants<float>::pi?1.0f:-1.0f);
     float x=((1.0f-oscMix)*s1+oscMix*s2+sub)*0.32f;
     x=filter.processSample(0,x);
     float e=env.getNextSample();
     float drive=1.0f+tubeDrive*8.0f+voltageAmt*2.0f;
     float bias=(voltageAmt-0.5f)*0.12f;
     x=(std::tanh((x+bias)*drive)-std::tanh(bias*drive))/std::max(1.0f,drive*0.58f);
     flux+=(x-flux)*0.0015f;
     float id=1.0f+ironAmt*4.0f;
     x=std::tanh((x+flux*ironAmt*0.2f)*id)/std::tanh(id);
     x*=e*level;
     for(int ch=0;ch<out.getNumChannels();++ch)out.addSample(ch,start+i,x);
     advance(phase1,f1);advance(phase2,f2);advance(subPhase,f1*0.5f);
     driftPhase+=0.00007f;if(driftPhase>juce::MathConstants<float>::twoPi)driftPhase-=juce::MathConstants<float>::twoPi;
   }
   if(!env.isActive())clearCurrentNote();
 }
private:
 double sampleRate=44100.0; float hz=440,level=0,bend=1,phase1=0,phase2=0,subPhase=0,driftPhase=0,flux=0;
 float oscMix=.5f,detune=7,cut=2200,reso=.35f,tubeDrive=.25f,ironAmt=.25f,voltageAmt=.35f;
 juce::ADSR env; juce::dsp::StateVariableTPTFilter<float> filter;
 float saw(float p){return p/juce::MathConstants<float>::pi-1.0f;}
 void advance(float& p,float f){p+=juce::MathConstants<float>::twoPi*f/(float)sampleRate;if(p>=juce::MathConstants<float>::twoPi)p-=juce::MathConstants<float>::twoPi;}
};
