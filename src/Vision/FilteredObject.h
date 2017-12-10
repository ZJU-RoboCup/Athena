// FilteredObject.h: interface for the FilteredObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined FILTEREDOBJECT_H
#define FILTEREDOBJECT_H

/**
  * Class FilteredObject
  * This filtered object class gives the predicted position and velocity using Kalman Filter
  * @author Chee Yong Lee
  * Developed from Matlab Code filtDesign written by Raffaello D'Andrea.
  * The updating equation is as follows :
  * filtOut   = Cimp*filtState + Dimp*posMeasure
  * filtState = Aimp*filtState + Bimp*posMeasure
  *
  * on loss of future information
  * filtState = inv(I-Aimp)Bimp + posMeasure, where we denote lossVec = inv(I-Aimp)Bimp
  * Date : June 2002
**/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
class FilteredObject{
public:
	/// Default constructor
	FilteredObject();
	/// Constructor with intialization matrices fed in
	FilteredObject(double A[2][2], double B[2], double C[2][2], double D[2],double lossMat[2]);
	/// Constructor from a file
	void initialize(const std::string& filename1, const std::string& filename2);
	/// Default Destructor
	virtual ~FilteredObject();
	/// Initialize the position whenever it is lost and refound. Use this for the first initial state too.
	void initializePos(double pos[2]);

	/** update the internal state using known vision data
	* @param z is the measured position. first component is for x axis, and second component is for y axis
	* @param filtOut is a pointer to whether the output position and velocity is output to
	*  First component is the dimension,  second component indicates whether it is position or velocity
	* @return filtered position and velocity in param filtOut
	**/
	void updatePosition(double z[2], double filtOut[2][2]);
	void reset() { usageCount = 0; }
private:
  
  /** FilterState : Internal representation for Kalman filter 
    * The first component represent the dimension. 0 for x axis, 1 for y axis
    * The second component represents the current position and velocity component
    * This is a column vector
   **/
  double filtState[2][2];
  double filtStateP[2][2];

  //./ filtered matrices that is determined by matlab
  double Aimp[2][2], Bimp[2], Cimp[2][2], Dimp[2], lossVec[2];
  double CimpInv[2][2];
  double AimpP[2][2], BimpP[2], CimpP[2][2], DimpP[2], lossVecP[2];

  /// The number of time it has been used
  long usageCount;
  double VELOCITY_THRES ;
};

#endif // !defined FILTEREDOBJECT_H
