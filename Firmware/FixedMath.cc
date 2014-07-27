#define FIXEDMUL(x,y,digits) ((x*y)>>digits)

long FixedSqrt(long S)
{
	
	long v;
	v = S/2;
	v = (v + S/v)/2;
	v = (v + S/v)/2;

	return v;
}
