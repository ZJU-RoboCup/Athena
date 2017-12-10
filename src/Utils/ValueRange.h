#ifndef _VALUE_RANGE_H_
#define _VALUE_RANGE_H_
#include <list>
#include <iostream>
#include <param.h> // add by jwl
#include <cmath>

class CValueRange{
public:
	CValueRange(double vMin, double vMax) : _vMin(vMin), _vMax(vMax),_vMinDist(0),_vMaxDist(0){ }
	CValueRange(double vMin, double vMax, double vMinDist, double vMaxDist): _vMin(vMin), _vMax(vMax),_vMinDist(vMinDist),_vMaxDist(vMaxDist){ }
	double getMax() const { return _vMax; }
	double getMin() const { return _vMin; }
	double getSize() const { return _vMax - _vMin; }
	double getMiddle() const { return (_vMax + _vMin) / 2 ;}
	void resize(double vMin, double vMax){ _vMin = vMin; _vMax = vMax; } 
	double getMinDist() const { return _vMinDist; }
	double getMaxDist() const { return _vMaxDist; }
	double getWidth() const { return std::sin(getSize()/2)*2*(std::min)(_vMinDist,_vMaxDist)*std::exp(-(std::min)(_vMinDist,_vMaxDist)/Param::Field::PITCH_LENGTH/4); } // 2*sin(alpha/2)*l*exp(-l/4/L)
	void resize(double vMin, double vMax, double vMinDist, double vMaxDist ){ _vMin = vMin; _vMax = vMax; _vMinDist = vMinDist; _vMaxDist = vMaxDist; } 
private:
	double _vMin;
	double _vMax;
	double _vMinDist; // shoot�������ϰ���ľ���
    double _vMaxDist; // 
};
class CValueRangeList{
typedef std::list< CValueRange > RangeList;
public:
	typedef RangeList::const_iterator iterator;
	void add(CValueRange range) // ����һ������
	{
		for( RangeList::iterator i=_rangeList.begin(); i!=_rangeList.end(); ){
			if( i->getMin() >= range.getMin() && i->getMax() <= range.getMax() ){
				// �µ��������ԭ������,ȥ��ԭ�е�
				_rangeList.erase(i++);
			}else if( i->getMin() <= range.getMin() && i->getMax() >= range.getMax() ){
				// ԭ�е��������������,�����������
				return;
			}else if(i->getMax() <= range.getMax() && i->getMax() >= range.getMin() ){
				// ���������ཻ,�ϲ�֮
				range = CValueRange(i->getMin(), range.getMax(), i->getMinDist(), range.getMaxDist());
				_rangeList.erase(i++);
			}else if( i->getMin() >= range.getMin() && i->getMin() <= range.getMax() ){
				// ���������ཻ,�ϲ�֮
				range = CValueRange(range.getMin(), i->getMax(), range.getMinDist(), i->getMaxDist());
				_rangeList.erase(i++);
			}else{
				++i;
			}
		}
		_rangeList.push_back(range);
	}
	void remove(const CValueRange& range) // ȥ��һ������
	{
		for( RangeList::iterator i=_rangeList.begin(); i!=_rangeList.end(); ){
			if( i->getMin() >= range.getMin() && i->getMax() <= range.getMax() ){
				// ��������Ҫȥ����������
				_rangeList.erase(i++);
			}else if( i->getMin() <= range.getMin() && i->getMax() >= range.getMax() ){
				// ���������Ҫȥ��������
				if( i->getMin() != range.getMin() ){
					_rangeList.push_back(CValueRange(i->getMin(), range.getMin(), i->getMinDist(), range.getMinDist()));
				}
				if( i->getMax() != range.getMax() ){
					_rangeList.push_back(CValueRange(range.getMax(), i->getMax(),range.getMaxDist(), i->getMaxDist()));
				}
				_rangeList.erase(i++);
			}else if( i->getMax() >= range.getMin() && i->getMax() <= range.getMax() ){
				// �������ཻ
				i->resize(i->getMin(), range.getMin(),i->getMinDist(), range.getMinDist());
				++i;
			}else if( i->getMin() >= range.getMin() && i->getMin() <= range.getMax() ){
				i->resize(range.getMax(), i->getMax(),i->getMinDist(), range.getMinDist());
				++i;
			}else{
				++i;
			}
		}
	}
	void removeAngleRange(const double rangeCenter, const double rangeHalfSize)
	{
		// rangeHalfҪ�㹻С�����ܴ���2PI
		const double angleMin = rangeCenter - rangeHalfSize;
		const double angleMax = rangeCenter + rangeHalfSize;
		if( angleMin < -Param::Math::PI ){
			remove(CValueRange(-Param::Math::PI, angleMax));
			remove(CValueRange(angleMin + Param::Math::PI * 2, Param::Math::PI));
		}else if( angleMax > Param::Math::PI ){
			remove(CValueRange(angleMin, Param::Math::PI));
			remove(CValueRange(-Param::Math::PI, angleMax - Param::Math::PI * 2));
		}else{
			remove(CValueRange(angleMin, angleMax));
		}
	}
	void removeAngleRange(const double rangeCenter, const double rangeHalfSize, const double dist)
	{
		// rangeHalfҪ�㹻С�����ܴ���2PI
		const double angleMin = rangeCenter - rangeHalfSize;
		const double angleMax = rangeCenter + rangeHalfSize;
		if( angleMin < -Param::Math::PI ){
			remove(CValueRange(-Param::Math::PI, angleMax, dist, dist));
			remove(CValueRange(angleMin + Param::Math::PI * 2, Param::Math::PI, dist, dist));
		}else if( angleMax > Param::Math::PI ){
			remove(CValueRange(angleMin, Param::Math::PI, dist, dist));
			remove(CValueRange(-Param::Math::PI, angleMax - Param::Math::PI * 2, dist, dist));
		}else{
			remove(CValueRange(angleMin, angleMax, dist, dist));
		}
	}
	size_t size() const { return _rangeList.size(); }
	iterator begin() const { return _rangeList.begin(); }
	iterator end() const { return _rangeList.end(); }
	void clear(){ _rangeList.clear(); }
	const CValueRange* getMaxRange() const
	{
		if( _rangeList.empty() ){
			return 0;
		}

		RangeList::const_iterator best=_rangeList.begin();
		for( RangeList::const_iterator i=++_rangeList.begin(); i!=_rangeList.end(); ++i ){
			if( i->getSize() > best->getSize() ){
				best = i;
			}
		}
		return &(*best);
	}
	const CValueRange* getMaxRangeWidth() const
	{
		if( _rangeList.empty() ){
			return 0;
		}

		RangeList::const_iterator best=_rangeList.begin();
		for( RangeList::const_iterator i=++_rangeList.begin(); i!=_rangeList.end(); ++i ){
			if( i->getWidth() > best->getWidth() ){
				best = i;
			}
		}
		return &(*best);
	}

	const CValueRange* getClosestRange(const double base, const double size) const
	{//����Ҫ��ֵ: ��Χ�������size,�����м�ֵ��base���(jwl)
		const CValueRange* result = 0;
		double min_dist = 0;
		for( RangeList::const_iterator i=_rangeList.begin(); i!=_rangeList.end(); ++i ){
			if( i->getSize() >= size ){ // ���ϴ�С
				if( result == 0 ){
					result = &(*i);
					min_dist = std::abs(i->getMiddle() - base);
				}else{
					double dist = std::abs(i->getMiddle() - base);
					if( dist < min_dist ){
						result = &(*i);
						min_dist = dist;
					}
				}
			}
		}
		return result;
	}

	const CValueRange* getClosestRangeWidth(const double base, const double size) const
	{//����Ҫ��ֵ: ��Χ�������size,�����м�ֵ��base���(Liunian)
		const CValueRange* result = 0;
		double min_dist = 0;
		for( RangeList::const_iterator i=_rangeList.begin(); i!=_rangeList.end(); ++i ){
			if( i->getWidth() >= size ){ // ���ϴ�С
				if( result == 0 ){
					result = &(*i);
					min_dist = std::abs(i->getMiddle() - base);
				}else{
					double dist = std::abs(i->getMiddle() - base);
					if( dist < min_dist ){
						result = &(*i);
						min_dist = dist;
					}
				}
			}
		}
		return result;
	}

	const CValueRange* findRange(const double value) const
	{
		for( RangeList::const_iterator i=_rangeList.begin(); i!=_rangeList.end(); ++i ){
			if( i->getMin() < value && i->getMax() > value ){
				return &(*i);
			}
		}
		return 0;
	}
	void print() const
	{
		for( RangeList::const_iterator i=_rangeList.begin(); i!=_rangeList.end(); ++i ){
			std::cout << " [" << i->getMin() << " -- " << i->getMax() << "] " << i->getWidth();
		}
		std::cout << Param::Output::NewLineCharacter;
	}
	CValueRangeList operator-(const CValueRangeList& valueRangeList) const
	{
		CValueRangeList resultList;
		for( iterator i=_rangeList.begin(); i!=_rangeList.end(); ++i){
			resultList.add(*i);
		}
		for( iterator i=valueRangeList.begin(); i!=valueRangeList.end(); ++i){
			resultList.remove(*i);
		}
		return resultList;
	}
private:
	RangeList _rangeList;
};
#endif // _VALUE_RANGE_H_
