#include "AmbienceNode.h"
#include "cinder/app/App.h"
#include "cinder/audio/audio.h"

using namespace ci;
using namespace std;

void AmbienceNode::initialize()
{
}

void AmbienceNode::process(ci::audio::Buffer *buffer)
{
	const size_t numFrames = buffer->getNumFrames();
	float *data = buffer->getData();

	for (size_t i = 0; i < numFrames; i++) {

		size_t channels = buffer->getNumChannels();
		for (size_t ch = 0; ch<channels; ch++) {
			
			data[ch*numFrames + i] = reverb(data[ch*numFrames + i], ch);
			//data[ch*numFrames + i] = data[ch*numFrames + i];


			float a, b, c, d, r;
			float t, f = fil, fb = fbak, dmp = damp, y = dry, w = wet;
			int p = pos, d1, d2, d3, d4;

			if (rdy == 0) suspend();

			d1 = (p + (VstInt32)(107 * size)) & 1023;
			d2 = (p + (VstInt32)(142 * size)) & 1023;
			d3 = (p + (VstInt32)(277 * size)) & 1023;
			d4 = (p + (VstInt32)(379 * size)) & 1023;


			while (--sampleFrames >= 0)
			{
				a = *++in1;
				b = *++in2;
				c = out1[1];
				d = out2[1]; //process from here...

				f += dmp * (w * (a + b) - f); //HF damping
				r = f;

				t = *(mBuf1 + p);
				r -= fb * t;
				*(mBuf1 + d1) = r; //allpass
				r += t;

				t = *(mBuf2 + p);
				r -= fb * t;
				*(mBuf2 + d2) = r; //allpass
				r += t;

				t = *(mBuf3 + p);
				r -= fb * t;
				*(mBuf3 + d3) = r; //allpass
				r += t;
				c += y * a + r - f; //left output

				t = *(mBuf4 + p);
				r -= fb * t;
				*(mBuf3 + d4) = r; //allpass
				r += t;
				d += y * b + r - f; //right output

				++p &= 1023;
				++d1 &= 1023;
				++d2 &= 1023;
				++d3 &= 1023;
				++d4 &= 1023;

				*++out1 = c;
				*++out2 = d;
			}
			pos = p;
			if (fabs(f)>1.0e-10) { fil = f;  den = 0; }  //catch denormals
			else { fil = 0.0f;  if (den == 0) { den = 1;  suspend(); } }

		}
	}
}

void AmbienceNode::setParameters() {

}

double AmbienceNode::reverb( double input, int ch ) {


}

void AmbienceNode::suspend()
{
	memset(mBuf1, 0, 1024 * sizeof(float));
	memset(mBuf2, 0, 1024 * sizeof(float));
	memset(mBuf3, 0, 1024 * sizeof(float));
	memset(mBuf4, 0, 1024 * sizeof(float));

	rdy = 1;
}