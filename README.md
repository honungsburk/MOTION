# Poco MD5 Example

Example of and MD5 calculation app using POCO C++ libraries installed with Conan C/C++ package manager.

Mostly used to run the conan.io getting started: https://docs.conan.io/en/latest/getting_started.html

## Compiling steps

1. Create a build directory:

    ```
    $ mkdir build && cd build
    ```

2. Install dependencies (Poco -> OpenSSL -> zlib):

    ```
    $ conan install ..
    ```

3. Configure the CMake project (Using MSVC 16 in this example):

    ```
    $ cmake .. -G "Visual Studio 16 2019"
    ```

4. Build it:

    ```
    $ cmake --build . --config Release
    ```

5. Run the application:

    ```
    $ .\bin\md5.exe
    c3fcd3d76192e4007dfb496cca67e13b
    ```


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // float vertices[vectorFieldDim * vectorFieldDim * 7 * 3];
    // unsigned int indices[vectorFieldDim * vectorFieldDim  * 9];

    // for (int i = 0; i < vectorFieldDim; i++) {
    //     for (int j = 0; j < vectorFieldDim; j++) {
    //         int verticesToCopy =  7 * 3;
    //         int  verticesBase = (i * vectorFieldDim + j) * verticesToCopy;
    //         float stepSize = 2.0f / float(vectorFieldDim - 1);
    //         glm::vec3 arrowStart, arrowEnd;
    //         arrowStart.x = float(i) * stepSize - 1.0f;
    //         arrowStart.y = float(j) * stepSize - 1.0f;
    //         arrowStart.z = 0.0;
    //         arrowEnd.x = arrowStart.x + vectorField[i][j].x * 0.1;
    //         arrowEnd.y = arrowStart.y + vectorField[i][j].y * 0.1;
    //         arrowEnd.z = 0.0;

    //         Arrow arrow(arrowStart, arrowEnd);

    //         int indicesToCopy = 9;
    //         int indicesBase = (i * vectorFieldDim + j) * indicesToCopy;

    //         arrow.copyTo(vertices, verticesBase, indices, indicesBase);
    //     }
    // }

    // unsigned int VBO, VAO, EBO;
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);

    // glBindVertexArray(VAO);

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    // // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // vectorFieldShader.use();
    // Defined the camera position
    // calculate the model matrix for each object and pass it to shader before drawing
    // glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    // model = glm::scale(model, glm::vec3(0.9, 0.9, 1.0));
    // // model = glm::translate(model, cubePositions[i]);
    // // float angle = 20.0f * i;
    // // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    // vectorFieldShader.setMat4("model", model);
