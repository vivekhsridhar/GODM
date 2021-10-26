Last updated: 10.2021 

Overview
--------

This repository is associated with the paper "The geometry of decision-making". 

Almost all animals must make decisions on the move. In this manuscript, we integrates theory and high-throughput experiments to reveal fundamental geometrical principles that result from the inherent interplay between movement and organisms’ internal representation of space. Specifically, we find that animals spontaneously reduce the world into a series of sequential binary decisions, a response that facilitates effective decision-making and is robust both to the number of options available, and to context, such as whether options are static (e.g. refuges) or mobile (e.g. other animals). We present evidence that these same principles, hitherto overlooked, apply across scales of biological organisation, from individual to collective decision-making.

Here, we provide all data and code required to reproduce the results presented in this paper.

Data
-----

All data required to reproduce our results are available in the [Data folder](./Data)
Within this, data is sorted by the experimental species, fruit flies, desert locusts, or larval zebrafish.

Analysis
---------

The [Analysis folder](./Analysis) consists of all code required to reproduce experimental figures from the paper. As with the data, the analyses are also sorted by species. Separate readme files are included within these folders for further information on how to use them. All code was written using [Python](https://www.python.org/) and [jupyter notebooks](https://jupyter.org/).

Theory
------------

The [Theory folder](./Theory) is subdivided into two folders: [NN](./Theory/NN) consisting of code to run the neural decision-making model, and [SPP](./Theory/SPP) consisting of code to run the collective decision-making model. Both models are written in C++ and use [OpenCV](https://opencv.org/) for visualisation.

Cite
----

If you use code from this repository, please cite it using this zenodo DOI:

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1134016.svg)](https://doi.org/10.5281/zenodo.1134016)


Thank You for using and citing us!
