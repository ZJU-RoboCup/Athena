#include "munkresTacticPositionMatcher.h"
#include "munkres.h"

namespace {
	const CVisionModule* _pVision = NULL;

	double getPotential(const CVisionModule* pVision, int vecNum, MunkresTacticPosition mtp) {
		// TODO ��ʱֻ���� ������ �ľ���
		CVector player2pos = pVision->OurPlayer(vecNum).Pos() - mtp._pos;
		return player2pos.mod2()/4.0;
	}

}

CMunkresTacticPositionMatcher::CMunkresTacticPositionMatcher(const CVisionModule* pVision, const Role2TacPosList& role2TacPosList, const VehNumList& vehNumList)
{
	_role2TacPosList = role2TacPosList;
	_vehNumList = vehNumList;	

	_row2Role2TacPosPairList.clear();
	_col2VehNumPairList.clear();
	_role2VehNumList.clear();

	_pVision = pVision;
}

CMunkresTacticPositionMatcher::~CMunkresTacticPositionMatcher()
{

}

void CMunkresTacticPositionMatcher::doMatch()
{
	// ��������Ԥ����
	inputHandle();

	// ����������ƥ��
	match();

	return ;
}

const Role2VehNumList& CMunkresTacticPositionMatcher::getMatchResult()
{
	return _role2VehNumList;
}

void CMunkresTacticPositionMatcher::inputHandle()
{
	// �����룺��ɫ��-ս����λ�� ת��Ϊ �к�-��ɫ��-ս����λ��
	_row2Role2TacPosPairList.clear();
	for (Role2TacPosList::const_iterator iter = _role2TacPosList.begin(); iter != _role2TacPosList.end(); ++ iter) {
		_row2Role2TacPosPairList.push_back(Row2Role2TacPosPair(iter-_role2TacPosList.begin(),*iter));
	}

	// �����룺���� ת��Ϊ �к�-����
	_col2VehNumPairList.clear();
	for (VehNumList::const_iterator iter = _vehNumList.begin(); iter != _vehNumList.end(); ++ iter) {
		_col2VehNumPairList.push_back(Col2VehNumPair(iter-_vehNumList.begin(),*iter));
	}

	return ;
}

void CMunkresTacticPositionMatcher::match()
{
	int nrows = _row2Role2TacPosPairList.size();
	int ncols = _col2VehNumPairList.size();

	Matrix<double> matrix(nrows, ncols);

	// Initialize matrix with random values.
	for ( int row = 0 ; row < nrows ; row++ ) {
		for ( int col = 0 ; col < ncols ; col++ ) {
			matrix(row,col) = getPotential(_pVision, _col2VehNumPairList[col].second, _row2Role2TacPosPairList[row].second.second);
		}
	}

	// Apply Munkres algorithm to matrix.
	Munkres m;
	m.solve(matrix);

	// Display if exists error
	for ( int row = 0 ; row < nrows ; row++ ) {
		int rowcount = 0;
		for ( int col = 0 ; col < ncols ; col++  ) {
			if ( matrix(row,col) == 0 )
				rowcount++;
		}
		if ( rowcount != 1 ){
		//	std::cerr << "Row " << row << " has " << rowcount << " columns that have been matched." << std::endl;
		}
	}

	for ( int col = 0 ; col < ncols ; col++ ) {
		int colcount = 0;
		for ( int row = 0 ; row < nrows ; row++ ) {
			if ( matrix(row,col) == 0 )
				colcount++;
		}
		if ( colcount != 1 ){
		//	std::cerr << "Column " << col << " has " << colcount << " rows that have been matched." << std::endl;
		}
	}

	// Match info store
	_role2VehNumList.clear();
	for ( int row = 0 ; row < nrows ; row++ ) {
		for ( int col = 0 ; col < ncols ; col++  ) {
			if ( matrix(row,col) == 0 ) {
				_role2VehNumList.push_back(Role2VehNumPair(_row2Role2TacPosPairList[row].second.first,_col2VehNumPairList[col].second));
				break;
			}
		}
	}

	return ;
}