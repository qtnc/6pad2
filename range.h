#ifndef _____RANGE_H_____1
#define _____RANGE_H_____1

template<class T> struct Range {
const T &start, &finish;
inline Range (const T& s, const T& f): start(s), finish(f) {}
inline const T& begin () const { return start; }
inline const T& end () const { return finish; }
};

template<class T> inline Range<T> range (const T& a, const T& b) { return Range<T>(a,b); }
template <class T> inline Range<T> range (const std::pair<T,T>& p) { return Range<T>(p.first, p.second); }

#endif
