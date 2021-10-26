Instructions
------------

Like the flyVR experiments, automating the locustVR experiments was also done using [SQLite databases](https://www.sqlite.org/index.html). However, unlike the flyVR experiments, there are four project, and experiment databases for the locustVR experiments. [flyProjects database](flyProjects.db) contains different randomisations which could be used to run the experiment. From these, all randomisations that were picked, for which experiments were conducted, are presented in the [flyExperiments database](flyExperiments.db).

To reproduce the various decision-making plots in the manuscript, run the [quantify_bifurcations.ipynb](quantify_bifurcations.ipynb). The required dependencies can be inferred from the first cell on this jupyter notebook. Please ensure you have all required dependencies while running this code.
