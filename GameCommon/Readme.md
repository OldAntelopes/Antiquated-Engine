GameCommon overview
----------------------

A set of (reasonably) generic C++ game components.
Each folder here represents a set of related code that is reasonably well encapsulated (bar a dependency on the Pub libs).

One particular set to note is 'Platform' : This is supposed to provide a standardised framework for the initialisation and update of the app that is not tied specifically to windows. 

List of GameCommon framework components
-----------------------------------------

- **Audio** : Wrapper for basic audio playback
- **Background** : For some reason it seemed useful to have a class that can do generic background graphic rendering
- **Console** : For text input and output
- **EntitySystem** : Component-based entity system. Uses factory macros to register new entity types. 
- **GenericDB** : Generic layer to standardise DB access. I never did much with DBs :)
- **HTTP** : Nice n simple HTTP request handling with callbacks. Just a thin curl wrapper really.
- **Landscape** : A massive old heightmap landscape renderer, uses shaders to generate textures and stuff like that
- **ParticleSystem** : Generic particle system classes for rendering and ya know, particles
- **Physics** : Uses bullet physics, can be linked to the entity system to provide basic physics behaviours
- **Platform** : Abstracts a standard framework for app initialisation, interactions and updates
- **RenderUtil** : Some handy interfaces for doing standard graphicy things like drawing lines in the 3d world, rendering a model to a texture etc.
- **Resources** : No idea
- **UI** : Where the Pub/Interface library provides the basic 2d drawing functions, UI wraps these up for standard UI components like buttons, dropdowns etc
- **Util** : Various basic regularly used utility functions, including cJSON.
- **WebServer** : A very primitive C++ http/web API server. Use macros to register endpoints and etc.




