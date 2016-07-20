#include "hogtrain.h"
#include "log.h"

HogTrain::HogTrain(vector<Mat> &pos, vector<Mat> &neg, HogParamList params): GeneralTrain(pos, neg) {
    // Generate the parameters combinations
    vector<int> temp;
    combineVector(params_, params, temp);
}

void HogTrain::run() {
    // Loop through the parameters
    for(uint i = 0; i < params_.size(); i++) {
        Trainer trainer;

        // Get HOG Parameters Configs
        HOGDescriptor hog = makeDescriptor(params_[i]);

        // Prepare pos and negatives samples
        Kfold<vector<SampleInfo>::const_iterator> kf = prepareSamples(pos_, neg_, hog);
//        vector<SampleInfo> samples;

//        TRAIN_LOG << "Starting HOG computing..." << endl;
//        for (vector<Mat>::iterator img = pos_.begin(); img < pos_.end(); img++) computeMultipleHog(*img, +1, hog, samples);
//        for (vector<Mat>::iterator img = neg_.begin(); img < neg_.end(); img++) computeMultipleHog(*img, -1, hog, samples);
//        TRAIN_LOG << "Finished HOG computing" << endl;

//        Kfold<vector<SampleInfo>::const_iterator> kf(5, samples.begin(), samples.end());

        // Cross-validation
        float acc;
        for (int fold = 0; fold != folds_; fold++) {
            Stats stat;

            HOGTRAIN_LOG << "Training Fold #" << fold+1 << endl;

            // Get the k fold
            vector<SampleInfo> train_data;
            vector<SampleInfo> test_data;
            kf.getFold(fold + 1, back_inserter(train_data), back_inserter(test_data));

            // Allocate the SVM parameters (labels)
            HOGTRAIN_LOG << "Preparing SVM parameters on fold #" << fold+1 << endl;
            vector<int> labels;
            vector<Mat> gradient_lst;
            for(vector<SampleInfo>::iterator it = train_data.begin(); it < train_data.end(); it++) {
                gradient_lst.push_back((*it).hog);
                labels.push_back((*it).type);
            }

            // Train a SVM using the actual HOGs
            HOGTRAIN_LOG << "Training SVM on fold #" << fold+1 << endl;
            Ptr<SVM> svm = trainer.trainSvm(gradient_lst, labels);

            // Configure the detector to use the trained SVM
            vector<float> detector = trainer.getSvmDetector(svm);
            hog.setSVMDetector(detector);

            // Test the actual setting using the test set
            HOGTRAIN_LOG << "Testing Fold #" << fold+1 << endl;
            stat.test(test_data, hog);

            // Save the accuracy
            acc += stat.get_accuracy();

            HOGTRAIN_LOG << "Accuracy of fold #" << fold+1 << ": " << acc << endl;
        }

        // If the average accuracy is better than the last parameter set, then save it
        acc = acc / folds_;
        if (acc > besthog_.acc) {
            besthog_.descriptor = hog;
            besthog_.acc = acc;

            HOGTRAIN_LOG << "New best HOG!" << endl;
            besthog_.print();
        }
    }
}

HogBest HogTrain::getBest() {
    return besthog_;
}

void HogTrain::combineVector(vector<vector<int> > &output_perms, const vector<vector<int> > &input, vector<int> &cur_perm, unsigned cur_row) {
    if (cur_row >= input.size()) {
        // This is where you have found a new permutation.
        // Do whatever you want with it.
        output_perms.push_back (cur_perm);
        return;
    }

    for (unsigned i = 0; i < input[cur_row].size(); ++i) {
        cur_perm.push_back (input[cur_row][i]);
        combineVector(output_perms, input, cur_perm, cur_row + 1);
        cur_perm.pop_back ();
    }
}
