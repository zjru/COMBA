/*
 *  COMBA
 *  Copyright (C) 2017  RCSL, HKUST
 *  
 *  ONLY FOR ACADEMIC USE, NOT FOR COMMERCIAL USE.
 *  
 *  Please use our tool at academic institutions and non-profit 
 *  research organizations for research use. 
 *  
 *  
 */


#include "Constant.h"
#include "Power.h"

bool is_power_of_two(int n)
{
	if(n==0)
		return false;
	else if(n==1)
		return true;
	else if(n%2)
		return false;
	else
		return is_power_of_two(n/2);
}

int closest_bound_power_two(int n)
{
	int m1=0;
	int m2=0;
	int m=0;
	if(n<0){
		n=-n;
	}
	for(int i=n;i>0;--i){
		bool is_power_two=is_power_of_two(i);
		if(is_power_two==true){
			m1=i;
			break;
		}
	}
	for(int i=n;i<inf;++i){
		bool is_power_two=is_power_of_two(i);
		if(is_power_two==true){
			m2=i;
			break;
		}
	}
	int delta_m1=n-m1;
	int delta_m2=m2-n;
	if(delta_m1>delta_m2){
		m=m2;
	}
	else{
		m=m1;
	}
	return m;
}

int get_power_of_two(int n)
{
	int power=1;
	if(n==1){
		power=0;
	}
	else if(is_power_of_two(n)==true){
		power+=get_power_of_two(n/2);
	}
	else{
		n--;
		power=get_power_of_two(n);
	}
	return power;
}
