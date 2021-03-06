#include "HetrickCV.hpp"

struct Dust : Module 
{
	enum ParamIds 
	{
		RATE_PARAM,
		BIPOLAR_PARAM,
		NUM_PARAMS
	};
	enum InputIds 
	{
		RATE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds 
	{
		DUST_OUTPUT,
		NUM_OUTPUTS
	};

	float lastDensity = 0.0;
	float densityScaled = 0.0;
	float threshold = 0.0;

	Dust() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) 
	{
		
	}

	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void Dust::step() 
{
	float densityInput = params[RATE_PARAM].value + inputs[RATE_INPUT].value;
	
	if(lastDensity != densityInput)
	{
		densityScaled = clampf(densityInput, 0.0f, 4.0f) / 4.0f;
		densityScaled = engineGetSampleRate() * powf(densityScaled, 3.0f);
		lastDensity = densityInput;
		threshold = (1.0/engineGetSampleRate()) * densityScaled;
	}

	const float noiseValue = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	if (noiseValue < threshold)
	{
		const bool bipolar = (params[BIPOLAR_PARAM].value == 0.0);

		if(bipolar)
		{
			const float scale = (threshold > 0.0f) ? 2.0f/threshold : 0.0f;
			outputs[DUST_OUTPUT].value = clampf(noiseValue * scale - 1.0f, -1.0f, 1.0f);
		}
		else
		{
			const float scale = (threshold > 0.0f) ? 1.0f/threshold : 0.0f;
			outputs[DUST_OUTPUT].value = clampf(noiseValue * scale, 0.0f, 1.0f);
		}
	}
	else
	{
		outputs[DUST_OUTPUT].value = 0.0;
	}
}


DustWidget::DustWidget() 
{
	auto *module = new Dust();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Dust.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(createParam<Davies1900hBlackKnob>(Vec(28, 87), module, Dust::RATE_PARAM, 0, 4.0, 0.0));
	addParam(createParam<CKSS>(Vec(37, 220), module, Dust::BIPOLAR_PARAM, 0.0, 1.0, 0.0));

	//////INPUTS//////
	addInput(createInput<PJ301MPort>(Vec(33, 146), module, Dust::RATE_INPUT));

	//////OUTPUTS//////
	addOutput(createOutput<PJ301MPort>(Vec(33, 285), module, Dust::DUST_OUTPUT));
}
