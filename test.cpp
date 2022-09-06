#include <iostream>
#include <string>
#include "pathmap.hpp"

using namespace std;

template<class T>
ostream& operator<<(ostream& os, set<T> const& xs) {
	os << "{ ";
	for( auto const& x : xs ) {
		os << x << ' ';
	}
	return os << '}';
}
template<class T>
ostream& operator<<(ostream& os, vector<T> const& xs) {
	os << "[ ";
	for( auto const& x : xs ) {
		os << x << ' ';
	}
	return os << ']';
}
template<typename L, typename T, class P = vector<L>>
ostream& operator<<(ostream& os, pathmap<L,T,P> const& m) {
	for( auto it = m.begin(); it != m.end(); it++ ) {
		os << "{ " << it.path() << "} : " << *it << endl;
	}
	return os;
}

int main() {
	setmap<int,string> m;
	m[{1,4}] = "foo";
	m[{6,4,2}] = "bar";
	m[{1}] = "baz";
	m[{1,4,6,2}] = "hoge";
	cout << m;
	m[{2,4,6}] = "bar2";
	cout << "====" << endl << m;
	cout << "supersets of {1}? " << m.supersets({1}) << endl;
}
