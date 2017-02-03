#include "MdisNacSensorModel.h"

#include <iomanip>
#include <iostream>

#include <csm/Error.h>

using namespace std;

const std::string MdisNacSensorModel::_SENSOR_MODEL_NAME
                                      = "ISIS_MDISNAC_USGSAstro_1_Linux64_csm30.so";



MdisNacSensorModel::MdisNacSensorModel() {

  m_transX[0] = 0.0;
  m_transX[1] = 0.0;
  m_transX[2] = 0.0;

  m_transY[0] = 0.0;
  m_transY[1] = 0.0;
  m_transY[2] = 0.0;

  m_majorAxis = 0.0;
  m_minorAxis = 0.0;
  m_omega = 0.0;
  m_phi = 0.0;
  m_kappa = 0.0;
  m_focalLength = 0.0;

  m_spacecraftPosition[0] = 0.0;
  m_spacecraftPosition[1] = 0.0;
  m_spacecraftPosition[2] = 0.0;

  m_ccdCenter = 0.0;

  m_line_pp = 0.0;
  m_sample_pp = 0.0;

#if 0
  //NAC coefficients
  m_odtX[0]=0.0;
  m_odtX[1]=1.0018542696237999756;
  m_odtX[2]=-0.0;
  m_odtX[3]=-0.0;
  m_odtX[4]=-0.00050944404749411103042;
  m_odtX[5]=0.0;
  m_odtX[6]=1.0040104714688599425e-05;
  m_odtX[7]=0.0;
  m_odtX[8]=1.0040104714688599425e-05;
  m_odtX[9]=0.0;

  m_odtY[0]=0.0;
  m_odtY[1]=0.0;
  m_odtY[2]=1.0;
  m_odtY[3]=0.00090600105949967496381;
  m_odtY[4]=0.0;
  m_odtY[5]=0.00035748426266207598964;
  m_odtY[6]=0.0;
  m_odtY[7]=1.0040104714688599425e-05;
  m_odtY[8]=0.0;
  m_odtY[9]=1.0040104714688599425e-05;
#endif
  m_odtX[0] = 0.0;
  m_odtX[1] = 0.0;
  m_odtX[2] = 0.0;
  m_odtX[3] = 0.0;
  m_odtX[4] = 0.0;
  m_odtX[5] = 0.0;
  m_odtX[6] = 0.0;
  m_odtX[7] = 0.0;
  m_odtX[8] = 0.0;
  m_odtX[9] = 0.0;

  m_odtY[0] = 0.0;
  m_odtY[1] = 0.0;
  m_odtY[2] = 0.0;
  m_odtY[3] = 0.0;
  m_odtY[4] = 0.0;
  m_odtY[5] = 0.0;
  m_odtY[6] = 0.0;
  m_odtY[7] = 0.0;
  m_odtY[8] = 0.0;
  m_odtY[9] = 0.0;


}


MdisNacSensorModel::~MdisNacSensorModel() {}


/**
 * @brief Compute undistorted focal plane x/y.
 *
 * Computes undistorted focal plane (x,y) coordinates given a distorted focal plane (x,y)
 * coordinate. The undistorted coordinates are solved for using the Newton-Raphson
 * method for root-finding if the distortionFunction method is invoked.
 *
 * @param dx distorted focal plane x in millimeters
 * @param dy distorted focal plane y in millimeters
 * @param undistortedX The undistorted x coordinate, in millimeters.
 * @param undistortedY The undistorted y coordinate, in millimeters.
 *
 * @return if the conversion was successful
 * @todo Review the tolerance and maximum iterations of the root-
 *       finding algorithm.
 * @todo Review the handling of non-convergence of the root-finding
 *       algorithm.
 * @todo Add error handling for near-zero determinant.
*/
bool MdisNacSensorModel::setFocalPlane(double dx,double dy,
                                       double &undistortedX,
                                       double &undistortedY ) const {


  // Solve the distortion equation using the Newton-Raphson method.
  // Set the error tolerance to about one millionth of a NAC pixel.
  const double tol = 1.4E-5;

  // The maximum number of iterations of the Newton-Raphson method.
  const int maxTries = 60;

  double x;
  double y;
  double fx;
  double fy;
  double Jxx;
  double Jxy;
  double Jyx;
  double Jyy;

  // Initial guess at the root
  x = dx;
  y = dy;

  distortionFunction(x, y, fx, fy);

  for (int count = 1; ((fabs(fx) + fabs(fy)) > tol) && (count < maxTries); count++) {

    this->distortionFunction(x, y, fx, fy);

    fx = dx - fx;
    fy = dy - fy;

    distortionJacobian(x, y, Jxx, Jxy, Jyx, Jyy);

    double determinant = Jxx * Jyy - Jxy * Jyx;
    if (determinant < 1E-6) {
      //
      // Near-zero determinant. Add error handling here.
      //
      //-- Just break out and return with no convergence
      break;
    }

    x = x + (Jyy * fx - Jxy * fy) / determinant;
    y = y + (Jxx * fy - Jyx * fx) / determinant;
  }

  if ( (fabs(fx) + fabs(fy)) <= tol) {
    // The method converged to a root.
    undistortedX = x;
    undistortedY = y;

  }
  else {
    // The method did not converge to a root within the maximum
    // number of iterations. Return with no distortion.
    undistortedX = dx;
    undistortedY = dy;
  }

  return true;

}


/**
 * @description Jacobian of the distortion function. The Jacobian was computed
 * algebraically from the function described in the distortionFunction
 * method.
 *
 * @param x
 * @param y
 * @param Jxx  Partial_xx
 * @param Jxy  Partial_xy
 * @param Jyx  Partial_yx
 * @param Jyy  Partial_yy
 */
void MdisNacSensorModel::distortionJacobian(double x, double y, double &Jxx, double &Jxy,
                                            double &Jyx, double &Jyy) const {

  double d_dx[10];
  d_dx[0] = 0;
  d_dx[1] = 1;
  d_dx[2] = 0;
  d_dx[3] = 2 * x;
  d_dx[4] = y;
  d_dx[5] = 0;
  d_dx[6] = 3 * x * x;
  d_dx[7] = 2 * x * y;
  d_dx[8] = y * y;
  d_dx[9] = 0;
  double d_dy[10];
  d_dy[0] = 0;
  d_dy[1] = 0;
  d_dy[2] = 1;
  d_dy[3] = 0;
  d_dy[4] = x;
  d_dy[5] = 2 * y;
  d_dy[6] = 0;
  d_dy[7] = x * x;
  d_dy[8] = 2 * x * y;
  d_dy[9] = 3 * y * y;

  Jxx = 0.0;
  Jxy = 0.0;
  Jyx = 0.0;
  Jyy = 0.0;

  for (int i = 0; i < 10; i++) {
    Jxx = Jxx + d_dx[i] * m_odtX[i];
    Jxy = Jxy + d_dy[i] * m_odtX[i];
    Jyx = Jyx + d_dx[i] * m_odtY[i];
    Jyy = Jyy + d_dy[i] * m_odtY[i];
  }


}


/**
 * @description Compute distorted focal plane (dx,dy) coordinate  given an undistorted focal
 * plane (ux,uy) coordinate. This describes the third order Taylor approximation to the
 * distortion model.
 *
 * @param ux Undistored x
 * @param uy Undistored y
 * @param dx Result distorted x
 * @param dy Result distorted y
 */
void MdisNacSensorModel::distortionFunction(double ux, double uy, double &dx, double &dy) const {

  double f[10];
  f[0] = 1;
  f[1] = ux;
  f[2] = uy;
  f[3] = ux * ux;
  f[4] = ux * uy;
  f[5] = uy * uy;
  f[6] = ux * ux * ux;
  f[7] = ux * ux * uy;
  f[8] = ux * uy * uy;
  f[9] = uy * uy * uy;

  dx = 0.0;
  dy = 0.0;

  for (int i = 0; i < 10; i++) {
    dx = dx + f[i] * m_odtX[i];
    dy = dy + f[i] * m_odtY[i];
  }

}

csm::ImageCoord MdisNacSensorModel::groundToImage(const csm::EcefCoord &groundPt,
                              double desiredPrecision,
                              double *achievedPrecision,
                              csm::WarningList *warnings) const {

double xl, yl, zl;
xl = m_spacecraftPoition[0];
yl = m_spacecraftPoition[1];
zl = m_spacecraftPoition[2];

double x, y, z;
x = groundPt.x;
y = groundPt.y;
z = groundPt.z;

double xo, yo, zo;
xo = xl - x;
yo = yl - y;
zo = zl - z;

double f;
f = m_focalLength

// Camera rotation matrix
double m[3][3];
calcRotationMatrix(m);

// Sensor position
double line, sample;

denom = m[0][2] * xo + m[1][2] * yo + m[2][2] * zo;
sample = (-f * (m[0][0] * xo + m[1][0] * yo + m[2][0] * zo)/denom) + m_sample_pp;
line = (-f * (m[0][1] * xo + m[1][1] * yo + m[2][1] * zo)/denom) + m_line_pp;

return csm:ImaggeCoord(line, sample);

}


csm::ImageCoordCovar MdisNacSensorModel::groundToImage(const csm::EcefCoordCovar &groundPt,
                                   double desiredPrecision,
                                   double *achievedPrecision,
                                   csm::WarningList *warnings) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::groundToImage");
}


csm::EcefCoord MdisNacSensorModel::imageToGround(const csm::ImageCoord &imagePt,
                                                 double height,
                                                 double desiredPrecision,
                                                 double *achievedPrecision,
                                                 csm::WarningList *warnings) const {

  // I do not see this shift in other CSM and adding it results in less accurate results.
  double sample = imagePt.samp;
  double line = imagePt.line;
  sample -= m_ccdCenter - 0.5; // ISD needs a center sample in CSM coord
  line -= m_ccdCenter - 0.5; // ISD needs a center line in CSM coord (.5 .5 pixel centers)

  //Here is where we should be able to apply an adjustment to opk
  double m[3][3];
  calcRotationMatrix(m);

  // Collinearity not solving for scale
  double xl, yl, zl, lo, so;
  lo = line - m_line_pp;
  so - line - m_sample_pp;


  // Apply the distortion model
  double dlo, dso;
  distortionFunction(lo, so, dlo, dso);

  xl = m[0][0] * dso + m[0][1] * dlo - m[0][2] * -m_focalLength;
  yl = m[1][0] * dso + m[1][1] * dlo - m[1][2] * -m_focalLength;
  zl = m[2][0] * dso + m[2][1] * dlo - m[2][2] * -m_focalLength;

  double x, y, z;
  double xc, yc, zc;
  xc = m_spacecraftPosition[0];
  yc = m_spacecraftPosition[1];
  zc = m_spacecraftPoition[2];
  // Intersect with some height about the ellipsoid.
  losEllipsoidIntersect(height, xc, yc, zc, xl, yl, zl, x, y, z);

  return csm::EcefCoord(x, y, z);

csm::EcefCoordCovar MdisNacSensorModel::imageToGround(const csm::ImageCoordCovar &imagePt, double height,
                                  double heightVariance, double desiredPrecision,
                                  double *achievedPrecision,
                                  csm::WarningList *warnings) const {
    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::imageToGround");
}

csm::EcefLocus MdisNacSensorModel::imageToProximateImagingLocus(const csm::ImageCoord &imagePt,
                                            const csm::EcefCoord &groundPt,
                                            double desiredPrecision,
                                            double *achievedPrecision,
                                            csm::WarningList *warnings) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::imageToProximateImagingLocus");
}


csm::EcefLocus MdisNacSensorModel::imageToRemoteImagingLocus(const csm::ImageCoord &imagePt,
                                         double desiredPrecision,
                                         double *achievedPrecision,
                                         csm::WarningList *warnings) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::imageToRemoteImagingLocus");
}


csm::ImageCoord MdisNacSensorModel::getImageStart() const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getImageStart");
}

csm::ImageVector MdisNacSensorModel::getImageSize() const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getImageSize");
}

std::pair<csm::ImageCoord, csm::ImageCoord> MdisNacSensorModel::getValidImageRange() const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getValidImageRange");
}

std::pair<double, double> MdisNacSensorModel::getValidHeightRange() const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getValidHeightRange");
}

csm::EcefVector MdisNacSensorModel::getIlluminationDirection(const csm::EcefCoord &groundPt) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getIlluminationDirection");
}

double MdisNacSensorModel::getImageTime(const csm::ImageCoord &imagePt) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getImageTime");
}

csm::EcefCoord MdisNacSensorModel::getSensorPosition(const csm::ImageCoord &imagePt) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getSensorPosition");
}

csm::EcefCoord MdisNacSensorModel::getSensorPosition(double time) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getSensorPosition");
}

csm::EcefVector MdisNacSensorModel::getSensorVelocity(const csm::ImageCoord &imagePt) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getSensorVelocity");
}

csm::EcefVector MdisNacSensorModel::getSensorVelocity(double time) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getSensorVelocity");
}

csm::RasterGM::SensorPartials MdisNacSensorModel::computeSensorPartials(int index, const csm::EcefCoord &groundPt,
                                           double desiredPrecision,
                                           double *achievedPrecision,
                                           csm::WarningList *warnings) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::computeSensorPartials");
}

csm::RasterGM::SensorPartials MdisNacSensorModel::computeSensorPartials(int index, const csm::ImageCoord &imagePt,
                                          const csm::EcefCoord &groundPt,
                                          double desiredPrecision,
                                          double *achievedPrecision,
                                          csm::WarningList *warnings) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::computeSensorPartials");
}

std::vector<double> MdisNacSensorModel::computeGroundPartials(const csm::EcefCoord &groundPt) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::computeGroundPartials");
}

const csm::CorrelationModel& MdisNacSensorModel::getCorrelationModel() const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getCorrelationModel");
}

std::vector<double> MdisNacSensorModel::getUnmodeledCrossCovariance(const csm::ImageCoord &pt1,
                                                const csm::ImageCoord &pt2) const {

    throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
      "Unsupported function",
      "MdisNacSensorModel::getUnmodeledCrossCovariance");
}




csm::Version MdisNacSensorModel::getVersion() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getVersion");
}


std::string MdisNacSensorModel::getModelName() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getModelName");
}


std::string MdisNacSensorModel::getPedigree() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getPedigree");
}


std::string MdisNacSensorModel::getImageIdentifier() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getImageIdentifier");
}


void MdisNacSensorModel::setImageIdentifier(const std::string& imageId,
                                            csm::WarningList* warnings) {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::setImageIdentifier");
}


std::string MdisNacSensorModel::getSensorIdentifier() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getSensorIdentifier");
}


std::string MdisNacSensorModel::getPlatformIdentifier() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getPlatformIdentifier");
}


std::string MdisNacSensorModel::getCollectionIdentifier() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getCollectionIdentifier");
}


std::string MdisNacSensorModel::getTrajectoryIdentifier() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getTrajectoryIdentifier");
}


std::string MdisNacSensorModel::getSensorType() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getSensorType");
}


std::string MdisNacSensorModel::getSensorMode() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getSensorMode");
}


std::string MdisNacSensorModel::getReferenceDateAndTime() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getReferenceDateAndTime");
}


std::string MdisNacSensorModel::getModelState() const {
  // TEMPORARY
  /* commented out for testing the gtest framework
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getModelState");
  */
  return "";
}


void MdisNacSensorModel::replaceModelState(const std::string& argState) {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::replaceModelState");
}




csm::EcefCoord MdisNacSensorModel::getReferencePoint() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getReferencePoint");
}


void MdisNacSensorModel::setReferencePoint(const csm::EcefCoord &groundPt) {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::setReferencePoint");
}


int MdisNacSensorModel::getNumParameters() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getNumParameters");
}


std::string MdisNacSensorModel::getParameterName(int index) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getParameterName");
}


std::string MdisNacSensorModel::getParameterUnits(int index) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getParameterUnits");
}


bool MdisNacSensorModel::hasShareableParameters() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::hasShareableParameters");
}


bool MdisNacSensorModel::isParameterShareable(int index) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::isParameterShareable");
}


csm::SharingCriteria MdisNacSensorModel::getParameterSharingCriteria(int index) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getParameterSharingCriteria");
}


double MdisNacSensorModel::getParameterValue(int index) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getParameterValue");
}


void MdisNacSensorModel::setParameterValue(int index, double value) {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::setParameterValue");
}


csm::param::Type MdisNacSensorModel::getParameterType(int index) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getParameterType");
}


void MdisNacSensorModel::setParameterType(int index, csm::param::Type pType) {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::setParameterType");
}


double MdisNacSensorModel::getParameterCovariance(int index1, int index2) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getParameterCovariance");
}


void MdisNacSensorModel::setParameterCovariance(int index1, int index2, double covariance) {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::setParameterCovariance");
}


int MdisNacSensorModel::getNumGeometricCorrectionSwitches() const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getNumGeometricCorrectionSwitches");
}


std::string MdisNacSensorModel::getGeometricCorrectionName(int index) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getGeometricCorrectionName");
}


void MdisNacSensorModel::setGeometricCorrectionSwitch(int index,
                                                      bool value,
                                                      csm::param::Type pType) {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::setGeometricCorrectionSwitch");
}


bool MdisNacSensorModel::getGeometricCorrectionSwitch(int index) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getGeometricCorrectionSwitch");
}


std::vector<double> MdisNacSensorModel::getCrossCovarianceMatrix(
    const GeometricModel &comparisonModel,
    csm::param::Set pSet,
    const GeometricModelList &otherModels) const {
  throw csm::Error(csm::Error::UNSUPPORTED_FUNCTION,
                   "Unsupported function",
                   "MdisNacSensorModel::getCrossCovarianceMatrix");
}

void MdisNacSensorModel::calcRotationMatrix(
  double m[3][3]) const {

  // Trigonometric functions for rotation matrix
  double sinw = std::sin(m_omega);
  double cosw = std::cos(m_omega);
  double sinp = std::sin(m_phi);
  double cosp = std::cos(m_phi);
  double sink = std::sin(m_kappa);
  double cosk = std::cos(m_kappa);

  // Rotation matrix taken from Introduction to Mordern Photogrammetry by
  // Edward M. Mikhail, et al., p. 373

  m[0][0] = cosp * cosk;
  m[0][1] = cosw * sink + sinw * sinp * cosk;
  m[0][2] = sinw * sink - cosw * sinp * cosk;
  m[1][0] = -1 * cosp * sink;
  m[1][1] = cosw * cosk - sinw * sinp * sink;
  m[1][2] = sinw * cosk + cosw * sinp * sink;
  m[2][0] = sinp;
  m[2][1] = -1 * sinw * cosp;
  m[2][2] = cosw * cosp;
}

void MdisNacSensorModel::losEllipsoidIntersect(
      const double& height,
      const double& xc,
      const double& yc,
      const double& zc,
      const double& xl,
      const double& yl,
      const double& zl,
      double&       x,
      double&       y,
      double&       z ) const
{
   // Helper function which computes the intersection of the image ray
   // with an expanded ellipsoid.  All vectors are in earth-centered-fixed
   // coordinate system with origin at the center of the earth.

   double ap, bp, k;
   ap = m__semiMajorAxis + height;
   bp = m_semiMinorAxis + height;
   k = ap * ap / (bp * bp);

   // Solve quadratic equation for scale factor
   // applied to image ray to compute ground point

   double at, bt, ct, quadTerm;
   at = xl * xl + yl * yl + k * zl * zl;
   bt = 2.0 * (xl * xc + yl * yc + k * zl * zc);
   ct = xc * xc + yc * yc + k * zc * zc - ap * ap;
   quadTerm = bt * bt - 4.0 * at * ct;

   // If quadTerm is negative, the image ray does not
   // intersect the ellipsoid. Setting the quadTerm to
   // zero means solving for a point on the ray nearest
   // the surface of the ellisoid.

   if ( 0.0 > quadTerm )
   {
      quadTerm = 0.0;
   }
   double scale;
   scale = (-bt - sqrt (quadTerm)) / (2.0 * at);

   // Compute ground point vector

   x = xc + scale * xl;
   y = yc + scale * yl;
   z = zc + scale * zl;
}
