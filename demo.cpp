#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/legacy/legacy.hpp>
#include <boost/random.hpp>
#include <boost/format.hpp>
#include "KAZE.h"
#include "config.h"
#include "utils.h"

toptions get_default_toptions(int w, int h)
{
    toptions options;
    options.soffset = DEFAULT_SCALE_OFFSET;
    options.omax = DEFAULT_OCTAVE_MAX;
    options.nsublevels = DEFAULT_NSUBLEVELS;
    options.dthreshold = DEFAULT_DETECTOR_THRESHOLD;
    options.diffusivity = DEFAULT_DIFFUSIVITY_TYPE;
    options.descriptor = DEFAULT_DESCRIPTOR_MODE;
    options.upright = DEFAULT_UPRIGHT;
    options.extended = DEFAULT_EXTENDED;
    options.sderivatives = DEFAULT_SIGMA_SMOOTHING_DERIVATIVES;
    options.save_scale_space = DEFAULT_SAVE_SCALE_SPACE;
    options.show_results = DEFAULT_SHOW_RESULTS;
    options.save_keypoints = DEFAULT_SAVE_KEYPOINTS;
    options.verbosity = DEFAULT_VERBOSITY;
    options.img_width = w;
    options.img_height = h;
    return options;
}

const float DRATIO = .6;		// NNDR Matching value
float Compute_Descriptor_Distance(Ipoint &p1, Ipoint &p2, float best)
{
    float dist = 0.0;
    int dsize = p1.descriptor_size;

    for(int i = 0; i < dsize; i++ )
    {
        dist += pow(p1.descriptor[i] - p2.descriptor[i],2);

        if( dist > best )
            break;
    }

    return dist;

} 

unsigned int Matching_Descriptor(std::vector<Ipoint> &ipts1, std::vector<Ipoint> &ipts2, std::vector<int> &indexes )
{
    float dist = 0.0, mind = 0.0, last_mind = 0.0;
    int mindex = -1;
    unsigned int correct_matches = 0;
    bool first = false;

    indexes.clear();

    for( unsigned int i = 0; i < ipts1.size(); i++ )
    {
        mind = 10000.0;
        last_mind = 10000.0;
        mindex = -1;
        first = false;

        for( unsigned int j = 0; j < ipts2.size(); j++ )
        {
            dist = Compute_Descriptor_Distance(ipts1[i],ipts2[j],1000.0);

            if( dist < mind )
            {
                if( first == false )
                {	mind = dist;
                    mindex = j;
                    first = true;
                }
                else
                {
                    last_mind = mind;
                    mind = dist;
                    mindex = j;
                }
            }
            else if( dist < last_mind )
            {
                last_mind = dist;
            }
        }

        if( mind < DRATIO*last_mind )
        {
            indexes.push_back(i);
            indexes.push_back(mindex);
            correct_matches++;
        }
    }

    return correct_matches;
}


std::vector<cv::KeyPoint> convert_Ipoint_Keypoint(const std::vector<Ipoint>& kps)
{
    std::vector<cv::KeyPoint> result;
    for(std::vector<Ipoint>::const_iterator p = kps.begin(); p != kps.end(); ++p)
        result.push_back(cv::KeyPoint(p->xf, p->yf, p->scale));
    return result;
}

std::vector<cv::DMatch> convert_KAZEMatch_DMatch(const std::vector<int>& match)
{
    std::vector<cv::DMatch> result;
    for(int i = 0; i < match.size(); i += 2)
        result.push_back(cv::DMatch(match[i], match[i + 1], 0));   //distanceはdummy
    return result;
}




int main(int argc, char *argv[])
{
    const int w = 640;
    const int h = 480;
    cv::Size size(w, h);
    cv::VideoCapture cap(0);
    if(!cap.isOpened())
        return 0;

    cv::Mat image;
    cv::Mat image_template = cv::Mat::zeros(size, CV_8UC1);
    cv::Mat img_match_sift, img_match_kaze;
    cv::namedWindow("result", 1);


    //initialize SIFT
    cv::SiftFeatureDetector sift_detector;
    cv::SiftDescriptorExtractor sift_extractor;
    cv::BruteForceMatcher<cv::L2<float> > matcher;
    std::vector<cv::KeyPoint> sift_kp_template, sift_kp_current;
    cv::Mat sift_template, sift_current;

    //initialize KAZE
    toptions kazeopt = get_default_toptions(w, h);
    KAZE kazeevol(kazeopt);
    cv::Mat image_32; //for kaze scalespace

    //KAZE feature extraction from source image
    std::vector<Ipoint> kaze_kp_template, kaze_kp_current;

    while(int key = cv::waitKey(30))
    {
        cap >> image;
        cv::resize(image, image, size);
        cv::cvtColor(image, image, CV_BGR2GRAY);

        if(key == ' ')
        {
            image_template = image.clone();

            //extract SIFT features from template image
            sift_detector.detect(image_template, sift_kp_template);
            sift_extractor.compute(image_template, sift_kp_template, sift_template);

            //extract KAZE features from template image
            image_template.convertTo(image_32, CV_32F, 1.0/255.0,0);
            kazeevol.Create_Nonlinear_Scale_Space(image_32);
            kazeevol.Feature_Detection(kaze_kp_template);
            kazeevol.Feature_Description(kaze_kp_template);

            continue;
        }

        //extract SIFT features from current image
        sift_detector.detect(image, sift_kp_current);
        sift_extractor.compute(image, sift_kp_current, sift_current);

        //match SIFT
        cv::vector<cv::DMatch> corr_sift;
        matcher.match(sift_template, sift_current, corr_sift);
        cv::drawMatches(image_template, sift_kp_template, image, sift_kp_current, corr_sift, img_match_sift);
        cv::putText(img_match_sift, (boost::format("SIFT: %d") % corr_sift.size()).str(), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 3);

        //extract KAZE features from current image
        image.convertTo(image_32, CV_32F, 1.0/255.0,0);
        kazeevol.Create_Nonlinear_Scale_Space(image_32);
        kazeevol.Feature_Detection(kaze_kp_current);
        kazeevol.Feature_Description(kaze_kp_current);

        //match KAZE
        std::vector<int> kaze_corr;
        Matching_Descriptor(kaze_kp_template, kaze_kp_current, kaze_corr);
        const std::vector<cv::DMatch> kaze_corr2 = convert_KAZEMatch_DMatch(kaze_corr);
        const std::vector<cv::KeyPoint> kaze_kp1 = convert_Ipoint_Keypoint(kaze_kp_template);
        const std::vector<cv::KeyPoint> kaze_kp2 = convert_Ipoint_Keypoint(kaze_kp_current);
        cv::drawMatches(image_template, kaze_kp1, image, kaze_kp2, kaze_corr2, img_match_kaze);
        cv::putText(img_match_kaze, (boost::format("KAZE: %d") % kaze_corr.size()).str(), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 3);

        //concat image
        cv::Mat result;
        cv::Mat src[] = {img_match_sift, img_match_kaze};
        cv::vconcat(src, 2, result);
        cv::imshow("result", result);
    }
}
