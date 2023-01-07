#include "fcl.h"
#include <math.h>

/**
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 *
 * (C) 2017-2022, Bernd Porr <bernd@glasgowneuro.tech>
 * (C) 2017, Paul Miller <paul@glasgowneuro.tech>
 **/

FeedforwardClosedloopLearning::FeedforwardClosedloopLearning(const int num_input,
	const TArray<int> &num_of_neurons_per_layer) {
#ifdef DEBUG
	fprintf(stderr, "Creating instance of FeedforwardClosedloopLearning.\n");
#endif	

	n_neurons_per_layer = num_of_neurons_per_layer;
	layers = new FCLLayer*[n_neurons_per_layer.Num()];
	ni = num_input;

	// creating input layer
	layers[0] = new FCLLayer(n_neurons_per_layer[0], ni);
	UE_LOG(LogTemp, Warning, TEXT("n[0]=%d"), n_neurons_per_layer[0]);

	for (int i = 1; i < n_neurons_per_layer.Num(); i++) {
		layers[i] = new FCLLayer(n_neurons_per_layer[i], layers[i - 1]->getNneurons());
		UE_LOG(LogTemp, Warning, TEXT("created layer %d with %d neurons"), i, layers[i]->getNneurons());
	}
	setLearningRate(0);
}

FeedforwardClosedloopLearning::~FeedforwardClosedloopLearning() {
	for (int i = 0; i < n_neurons_per_layer.Num(); i++) {
		delete layers[i];
	}
	delete[] layers;
}


void FeedforwardClosedloopLearning::setStep() {
	for (int k = 0; k < n_neurons_per_layer.Num(); k++) {
		layers[k]->setStep(step);
	}
}

void FeedforwardClosedloopLearning::setActivationFunction(FCLNeuron::ActivationFunction _activationFunction) {
	for (int k = 0; k < n_neurons_per_layer.Num(); k++) {
		layers[k]->setActivationFunction(_activationFunction);
	}
}

void FeedforwardClosedloopLearning::doLearning() {
	for (int k = 0; k < n_neurons_per_layer.Num(); k++) {
		layers[k]->doLearning();
	}
}


void FeedforwardClosedloopLearning::setDecay(double decay) {
	for (int k = 0; k < n_neurons_per_layer.Num(); k++) {
		layers[k]->setDecay(decay);
	}
}


void FeedforwardClosedloopLearning::doStep(const TArray<double>& input, const TArray<double>& error) {
/*	if (input.Num() != ni) {
		char tmp[256];
		sprintf(tmp, "Input array dim mismatch: got: %ld, want: %d.", (long)input.Num(), ni);
#ifdef DEBUG
		fprintf(stderr, "%s\n", tmp);
#endif
		throw tmp;
	}
	if (error.Num() != ni) {
		char tmp[256];
		sprintf(tmp,
			"Error array dim mismatch: got: %ld, want: %d "
			"which is the number of neurons in the 1st hidden layer!",
			(long)error.Num(), layers[0]->getNneurons());
#ifdef DEBUG
		fprintf(stderr, "%s\n", tmp);
#endif
		throw tmp;
	}*/
	// we set the input to the input layer
	layers[0]->setInputs((double *) input.GetData());
	// ..and calc its output
	layers[0]->calcOutputs();
	// new lets calc the other outputs
	for (int k = 1; k < n_neurons_per_layer.Num(); k++) {
		FCLLayer* emitterLayer = layers[k - 1];
		FCLLayer* receiverLayer = layers[k];
		// now that we have the outputs from the previous layer
		// we can shovel them into the next layer
		for (int j = 0; j < emitterLayer->getNneurons(); j++) {
			// get the output of a neuron in the input layer
			double v = emitterLayer->getNeuron(j)->getOutput();
			// set that output as an input to the next layer which
			// is distributed to all neurons
			receiverLayer->setInput(j, v);
		}

		// now let's calc the output which can then be sent out
		receiverLayer->calcOutputs();
	}
	// the error is injected into the 1st layer!
	for (int i = 0; i < (layers[0]->getNneurons()); i++) {
		layers[0]->getNeuron(i)->setError(error[i]);
	}
	for (int k = 1; k < n_neurons_per_layer.Num(); k++) {
		FCLLayer* emitterLayer = layers[k - 1];
		FCLLayer* receiverLayer = layers[k];
		// Calculate the errors for the hidden layer
		for (int i = 0; i < receiverLayer->getNneurons(); i++) {
			double err = 0;
			for (int j = 0; j < emitterLayer->getNneurons(); j++) {
				err = err + receiverLayer->getNeuron(i)->getWeight(j) *
					emitterLayer->getNeuron(j)->getError();
			}
			err = err * learningRateDiscountFactor;
			err = err * emitterLayer->getNneurons();
			err = err * receiverLayer->getNeuron(i)->dActivation();
			receiverLayer->getNeuron(i)->setError(err);
		}
	}
	doLearning();
	setStep();
	step++;
}

void FeedforwardClosedloopLearning::setLearningRate(double rate) {
	for (int i = 0; i < n_neurons_per_layer.Num(); i++) {
		layers[i]->setLearningRate(rate);
	}
}

void FeedforwardClosedloopLearning::setMomentum(double momentum) {
	for (int i = 0; i < n_neurons_per_layer.Num(); i++) {
		layers[i]->setMomentum(momentum);
	}
}



void FeedforwardClosedloopLearning::initWeights(double max, int initBias, FCLNeuron::WeightInitMethod weightInitMethod) {
	for (int i = 0; i < n_neurons_per_layer.Num(); i++) {
		layers[i]->initWeights(max, initBias, weightInitMethod);
	}
}


void FeedforwardClosedloopLearning::setBias(double _bias) {
	for (int i = 0; i < n_neurons_per_layer.Num(); i++) {
		layers[i]->setBias(_bias);
	}
}

void FeedforwardClosedloopLearning::setDebugInfo() {
	for (int i = 0; i < n_neurons_per_layer.Num(); i++) {
		layers[i]->setDebugInfo(i);
	}
}

// need to add bias weight
bool FeedforwardClosedloopLearning::saveModel(const char* name) {
	return true;
}

bool FeedforwardClosedloopLearning::loadModel(const char* name) {

	return true;
}
