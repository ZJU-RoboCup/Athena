/*========================================================================
    KDTree.h : Template for fast online creation KD Trees
  ------------------------------------------------------------------------
    Copyright (C) 1999-2002  James R. Bruce
    School of Computer Science, Carnegie Mellon University
  ------------------------------------------------------------------------
    This software is distributed under the GNU General Public License,
    version 2.  If you do not have a copy of this licence, visit
    www.gnu.org, or write: Free Software Foundation, 59 Temple Place,
    Suite 330 Boston, MA 02111-1307 USA.  This program is distributed
    in the hope that it will be useful, but WITHOUT ANY WARRANTY,
    including MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  ========================================================================*/

#ifndef __KD_TREE_H__
#define __KD_TREE_H__

#include "fast_alloc.h"
#include "vector.h"
#include <GDebugEngine.h>

#define KDT_TEMP template <class state>
#define KDT_FUN  KDTree<state>

// KDTree: ֻ��Ҷ�ڵ�Żᱣ��stateλ����Ϣ,����Ҷ�ڵ�ֻ�ǶԳ��ؽ��з���ʽ�Ļ���,û��ʵ�ʵ�λ�õ�ֲ�;
KDT_TEMP
class KDTree{
	struct node{
		vector2f minv,maxv; // bounding box of subtree
		state *states;      // list of states stored at this node
		int num_states;     // number of states of this subtree

		union{
			node *child[2]; // children of this tree
			node *next;
		};
	};

	node *root;
	int leaf_size,max_depth;
	int tests;
	fast_allocator<node> anode;

protected:
	inline bool inside(vector2f &minv,vector2f &maxv,state &s);
	inline float box_distance(vector2f &minv,vector2f &maxv,vector2f &p);

	void split(node *t,int split_dim);
	state *nearest(node *t,state *best,float &best_dist,vector2f &x);
	void clear(node *t);

	void drawATree(node* anode);
public:
	KDTree() {root=NULL; leaf_size=max_depth=0;}
    
	bool setdim(vector2f &minv,vector2f &maxv,int nleaf_size,int nmax_depth);
	bool add(state *s);
	void clear();
	state *nearest(float &dist,vector2f &x);

public:
	void drawTree();
};

// �ж�s�Ƿ��ڷ���(minv,maxv)֮��
KDT_TEMP
inline bool KDT_FUN::inside(vector2f &minv,vector2f &maxv,state &s)
{
	return(s.pos.x>minv.x && s.pos.y>minv.y 
		&& s.pos.x<maxv.x && s.pos.y<maxv.y);
}

// p�뽫��p�����ڷ���(minv,maxv)ʱ�Ķ�Ӧ��֮��ľ���
KDT_TEMP
inline float KDT_FUN::box_distance(vector2f &minv,vector2f &maxv,vector2f &p)
{
	float dx,dy;

	dx = p.x - bound(p.x,minv.x,maxv.x);
	dy = p.y - bound(p.y,minv.y,maxv.y);

	return(sqrt(dx*dx + dy*dy));
}

// ���ڵ�t����states����ά��split_dim���ж���,�����õ�������������(������㲻��states);
KDT_TEMP
void KDT_FUN::split(node *t,int split_dim)
{
	node *a,*b;
	state *p,*n;
	float split_val;

	// make new nodes
	a = anode.alloc();
	b = anode.alloc();
	if(!a || !b) return;

	a->child[0] = b->child[0] = NULL;
	a->child[1] = b->child[1] = NULL;
	a->states = b->states = NULL;
	a->num_states = b->num_states = 0;

	// determine split value
	a->minv = b->minv = t->minv;
	a->maxv = b->maxv = t->maxv;

	if(split_dim == 0){
		split_val = (t->minv.x + t->maxv.x) / 2;
		a->maxv.x = b->minv.x = split_val;
	}else{
		split_val = (t->minv.y + t->maxv.y) / 2;
		a->maxv.y = b->minv.y = split_val;
	}

	// ��t�ڵ�������states���з���,����a��bȥ;
	// separate children based on split_val
	n = t->states;
	while(p = n){
		n = n->next;

		// ��p��ֵ��split_val�Ƚ�,��Ӧ�ķֵ�a������b��������ȥ
		if(((split_dim == 0) ? p->pos.x : p->pos.y) < split_val){
			// ��p���뵽a����
			p->next = a->states;
			a->states = p;
			a->num_states++;
		}else{
			// ���뵽b����
			p->next = b->states;
			b->states = p;
			b->num_states++;
		}
	}

	// ��a,b�����嵽t����
	// insert into tree
	t->states = NULL;	// ������states,����states����child[0,1]����
	t->child[0] = a;
	t->child[1] = b;
}

// ����һ����
KDT_TEMP
bool KDT_FUN::setdim(vector2f &minv,vector2f &maxv,int nleaf_size,int nmax_depth)
{
	clear();
	if(!root) root = anode.alloc();
	if(!root) return(false);
	mzero(*root);
	root->minv = minv;
	root->maxv = maxv;
	leaf_size = nleaf_size;
	max_depth = nmax_depth;
	return(true);
}

KDT_TEMP
bool KDT_FUN::add(state *s)
{
	node *p;
	int c,level;

	level = 0;
	p = root;

	// ��Ϊ��,����λ��s���������ڵ�������ķ��η�Χ��,�򷵻�false
	if(!p || !inside(p->minv,p->maxv,*s)) return(false);

	// go down tree to see where new state should go
	while(p->child[0]){ // implies p->child[1] also
		c = !inside(p->child[0]->minv,p->child[0]->maxv,*s);	// s�Ƿ��������ڵ�0����ķ�������: ������������0,������; ����ȥ����1�м�����; һֱ�ҵ�Ҷ�ڵ�Ϊֹ;
		p = p->child[c];
		level++;
	}

	// ��s���뵽Ҷ�ڵ�״̬�б��ͷ��
	// add it to leaf; and split leaf if too many children
	s->next = p->states;
	p->states = s;
	p->num_states++;

	// split leaf if not too deep and too many children for one node
	if(level<max_depth && p->num_states>leaf_size){
		// ����level����ż�Զ�p���states����xά�Ȼ�yά�ȵĲ��
		split(p,level % 2);
	}
	return(true);
}


// ��t�������
KDT_TEMP
void KDT_FUN::clear(node *t)
{
	if(!t) return;
	if(t->child[0]) clear(t->child[0]);
	if(t->child[1]) clear(t->child[1]);

	t->child[0] = t->child[1] = NULL;
	t->states = NULL;
	t->num_states = 0;

	anode.free(t);
}

KDT_TEMP
void KDT_FUN::clear()
{
	if(!root) return;

	clear(root->child[0]);
	clear(root->child[1]);

	root->child[0] = root->child[1] = NULL;
	root->states = NULL;
	root->num_states = 0;
}

KDT_TEMP
state *KDT_FUN::nearest(node *t,state *best,float &best_dist,vector2f &x)
{
	float d,dc[2];
	state *p;
	int c;

	// look at states at current node
	// ��Ѱ��t�ڵ��ϵ�����state,�ҵ���С��state
	p = t->states;
	while(p){
		d = Vector::distance(p->pos,x);
		if(d < best_dist){
			best = p;
			best_dist = d;
		}
		tests++;
		p = p->next;
	}

	// �ݹ��Ѱ��t���������Ƿ��и����ĵ�
	// recurse on children (nearest first to maximize pruning)
	if(t->child[0]){ // implies t->child[1]
		dc[0] = box_distance(t->child[0]->minv,t->child[0]->maxv,x);
		dc[1] = box_distance(t->child[1]->minv,t->child[1]->maxv,x);

		// ����dc[0],dc[1]�ĸ�С,��λ��x���ĸ��ӷ������,���ж��ǿ��������и�Сֵ
		c = dc[1] < dc[0]; // c indicates nearest lower bound distance child

		if(dc[ c] < best_dist) best = nearest(t->child[ c],best,best_dist,x);
		if(dc[!c] < best_dist) best = nearest(t->child[!c],best,best_dist,x);
	}

	return(best);
}

KDT_TEMP
state *KDT_FUN::nearest(float &dist,vector2f &x)
{
	state *best;

	best = NULL;
	dist = 4000;

	tests = 0;
	best = nearest(root,best,dist,x);
	// printf("tests=%d dist=%f\n\n",tests,best_dist);

	return(best);
}

// recursively draw a tree
KDT_TEMP
void KDT_FUN::drawATree(node* anode)
{
	if (anode->states){
		std::cout<<"leaf start draw"<<endl;
		state* p = anode->states;
		while( p != NULL && p->next != NULL){
			GDebugEngine::Instance()->gui_debug_line(CGeoPoint(p->pos.x, p->pos.y), CGeoPoint(p->next->pos.x,p->next->pos.y), COLOR_ORANGE);
			p = p->next;
		}
		std::cout<<"leaf end draw"<<endl;
	}
	else if (anode->child[0]){
		std::cout<<"have child..."<<endl;
		drawATree(anode->child[0]);
		drawATree(anode->child[1]);
	}else{
		std::cout<<"wrong layer"<<std::endl;
		return;
	}
}

// �����������ĺ���
KDT_TEMP
void KDT_FUN::drawTree()
{
	drawATree(root);
}

#endif /*__KD_TREE_H__*/
