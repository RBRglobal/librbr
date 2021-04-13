# libRBR/dyncorr

## Introduction

libRBR provides an interface for performing dynamic correction of
measurements using Lueck-Picklo algorithm.


For example:

~~~{.c}
    RBR_DynCorrMeasurement  measIn;
    RBR_DynCorrMeasurement  MeasCorr;    
    RBR_DynCorrParams       params;
    RBR_DynCorrError        status;

    /* initialiaze the algorithm */
    status = RBRDynCorr_init(&params, Fs);
    if ( status != RBR_DCORR_SUCCESS )
    {
        printf("RBRDynCorr_init() return error code %u", status);
        return;
    }

    /* read measurement and perform a correction */
    while ( 1 )
    {
        /* input to algorithm */
        measIn.timestamp = get_timestamp();
        measIn.conductivity = get_conductivity();
        measIn.marineTemperature = get_temperature();
        measIn.condTemperature = get_cond_temperature();
        measIn.pressure = get_pressure();
        
        status = RBRDynCorr_addMeasurement(&params, &measIn, &MeasCorr);

        /* wait until sufficient sample feed into algorithm */
        if ( status == RBR_DCORR_NOT_VALID_YET )
        {
            continue;
        }

        /* check for error */
        if ( status != RBR_DCORR_SUCCESS )
        {
            MeasCorr.conductivity = NAN;
            MeasCorr.marineTemperature = NAN;
            MeasCorr.condTemperature = NAN;
            MeasCorr.salinity = NAN;
        }

        /* Otherwise, the MeasCorr struct hold the corrected measurement */
    }
~~~


## Dynamic correction

The correction is based on an algorithm from Lueck and Picko [1,2,3] to correction measurements
based on thermal lag between conductivity sensor temperature and marine temperature.


Calculation:

    params->_lagIndex = (int)(Fs * params->t_delay);
    params->_phi = (params->t_delay - params->_lagIndex/Fs)*Fs;


    nshift := floor(0.35 * Fs)
    phi := (0.35*Fs - nshift)
    Tshiftedn := T[nshift-1]*(1 - phi) + T[nshift]*phi

    Tshiftedn := Tn + (Tnshift - Tn)*(0.35*Fs - nshift)


Only input for initialization of algorithm is the sampling rate.  Other parameters are fixed and configured at compile-time  Initialization stage will pre-calculate some conversion factor for the given sampling rate, and initialize an array of time-shifted data for the calculation (invalid values used for error tracking, those values will never be used by the algorithm).  If the sampling rate change during data acquisition the algorithm need to be reinitialized with the new sampling rate.

Note the algorithm rely on static memory allocation, so only a limited of time-shifted data can be keep.  Initialization procedure will fail if the requested sampling rate would require more memory than pre-allocated amount(default value allow a maximum sampling rate of 50Hz sampling rate, which is faster than current capabilities of RBR CDT sensor).

RBRDynCorr_addMeasurement is called to inject a new measurement into the algorithm.  This will enter the new measurement data and use time-shifted data for calculation.  Tcor is calculated using a linear interpolation between last two temperature measurement.  Rest of calculation i On initial run, sufficient data need to be supplied to obtain a result.Function will return RBR_DCORR_NOT_VALID_YET for the first few samples before sufficient data are available to calculate Tcor. Once data is available,the function will return RBR_DCORR_SUCCESS and data will be output to the provided array.

Data need to be feed at the proper rate as defined by the sampling rate, otherwise the incorrect number of samples will be shifted and an incorrect result will be produced.Sanity check is done on the data to make sure an invalid value (not-a-number) cannot be injected into the recursive equation, as it will not decay away.


[1] Lueck R.G., Journal of Atmosheric and Oceanic Technology, 1990, vol 7, 741-755

[2] Lueck R.G. and J.Picklo, Journal of Atmosheric and Oceanic Technology, 1990, vol 7, 756-786

[3] Morisson J et al, Journal of Atmosheric and Oceanic Technology, 1994, vol 11, 1151-1164


## Contributing

The library is primarily maintained by RBR
and development is directed by our needs
and the needs of our [OEM] customers.
However, we're happy to take [contributions] generally.

[OEM]: https://rbr-global.com/products/oem
[contributions]: CONTRIBUTING.md

## License

This project is licensed under the terms
of the Apache License, Version 2.0;
see https://www.apache.org/licenses/LICENSE-2.0.

* The license is not “viral”.
  You can include it
  either as source
  or by linking against it,
  statically or dynamically,
  without affecting the licensing
  of your own code.
* You do not need to include RBR's copyright notice
  in your documentation,
  nor do you need to display it
  at program runtime.
  You must retain RBR's copyright notice
  in library source files.
* You are under no legal obligation
  to share your own modifications
  (although we would appreciate it
  if you did so).
* If you make changes to the source,
  in addition to retaining RBR's copyright notice,
  you must add a notice stating that you changed it.
  You may add your own copyright notices.
