/*! file gettingStarted.cpp
    Created by David Daverio.

    A simple example of LATfield2 usage.
 */


#include "LATfield2.hpp"
using namespace LATfield2;


int main(int argc, char **argv)
{

    //-------- Initilization of the parallel object ---------
    int n,m;

    for (int i=1 ; i < argc ; i++ ){
		if ( argv[i][0] != '-' )
			continue;
		switch(argv[i][1]) {
			case 'n':
				n = atoi(argv[++i]);
				break;
			case 'm':
				m =  atoi(argv[++i]);
				break;
		}
	}

    parallel.initialize(n,m);

    COUT << "Parallel grid size: ("<<parallel.grid_size()[0]<<","<<parallel.grid_size()[1]<<"). "<<endl;
    //-----------------------   end   ------------------------


    //------------   Declaration of a Lattice   --------------

    int dim = 3;
    int latSize[3] = {64,64,64};
    int halo = 1;
    Lattice lat(dim,latSize,halo);

    COUT << "Lattice size: ("<< lat.size(0)<<","<< lat.size(1)<<","<< lat.size(2)<<");"<<endl;
    cout << "Process ranks: "<< parallel.rank()<<",("<< parallel.grid_rank()[0]<<","<<parallel.grid_rank()[1]<< "); ";
    cout << "Local lattice size: ("<< lat.sizeLocal(0)<<","<< lat.sizeLocal(1)<<","<< lat.sizeLocal(2)<<"); ";
    cout << "Coordinate of the first local point: (0,"<< lat.coordSkip()[1] <<","<< lat.coordSkip()[0] <<")."<<endl;
    //-----------------------   end   ------------------------


    //-----------   Declaration of the Fields   --------------

    Field<Real> rho(lat);
    Field<Real> gradPhi(lat,3);

    Field<Real> phi;
    phi.initialize(lat);
    phi.alloc();
    //-----------------------   end   ------------------------

    //--------------   Operations on Fields   ----------------
    Site x(lat);

    double x2;




    for(x.first();x.test();x.next())
    {
        x2  = pow(0.5 + x.coord(0) - lat.size(0)/2.,2.);
        x2 += pow(0.5 + x.coord(1) - lat.size(1)/2.,2.);
        x2 += pow(0.5 + x.coord(2) - lat.size(2)/2.,2.);
        phi(x) = 1.0 + x.coord(0);//exp(-x2 * 2.);
    }

    phi.updateHalo();

    for(x.first();x.test();x.next())
    {

        gradPhi(x,0) = (phi(x+0)-phi(x-0));
        gradPhi(x,1) = (phi(x+1)-phi(x-1));
        gradPhi(x,2) = (phi(x+2)-phi(x-2));

        rho(x)=0;
        for(int i=0;i<3;i++)rho(x) += phi(x+i) - 2 * phi(x) + phi(x-i);

    }

    long index;
    #pragma omp parallel for collapse(1) private(index)
    for(int k  = halo; k< halo + lat.sizeLocal(2);k++)
      for(int j  = halo; j< halo + lat.sizeLocal(1);j++)
        for(int i  = halo; i< halo + lat.sizeLocal(0);i++)
    {
        index  = i + lat.sizeLocalGross(0) * (j + lat.sizeLocalGross(1) * k);//i + lat.sizeLocalGross(0) * (j + lat.sizeLocalGross(1)*k);
        gradPhi(index,0) = 1;//(phi(index+lat.jump(0))-phi(index-lat.jump(0)));
        gradPhi(index,1) = 1;//(phi(index+lat.jump(1))-phi(index-lat.jump(1)));
        gradPhi(index,2) = 1;//(phi(index+lat.jump(2))-phi(index-lat.jump(2)));

        rho(index)=0;
        for(int i=0;i<3;i++)rho(index) += phi(index+lat.jump(i)) - 2 * phi(index) + phi(index-lat.jump(i));
    }


    for(int k  = halo; k< halo + lat.sizeLocal(2);k++)
      for(int j  = halo; j< halo + lat.sizeLocal(1);j++)
	//#pragma omp parallel for collapse(1) private(x)
        for(int i  = halo; i< halo + lat.sizeLocal(0);i++)
      {
          x.setIndex(i + lat.sizeLocalGross(0) * (j + lat.sizeLocalGross(1)*k));

          gradPhi(x,0) = (phi(x+0)-phi(x-0));
          gradPhi(x,1) = (phi(x+1)-phi(x-1));
          gradPhi(x,2) = (phi(x+2)-phi(x-2));

          rho(x)=0;
          for(int i=0;i<3;i++)rho(x) += phi(x+i) - 2 * phi(x) + phi(x-i);
      }






}
