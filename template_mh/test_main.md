## Explicación de los Bloques de Test

**Bloque SETUP:** Intenta instanciar la clase. Si el lector CSV falla (por ejemplo, si te equivocas en la ruta `filepath`), lanzará una excepción y el texto rojo te avisará inmediatamente de que no encuentra el fichero.

**Bloque TEST 1:** Valida que el `CsvReader` haya contado las columnas correctamente.

**Bloque TEST 2:** Es la prueba de fuego de tu función `fix`. Suma todos los valores (para ver si da `1.0`), cuenta los ceros (para asegurar la diversificación) y verifica que ningún número viole los rangos `lo` y `hi`.

**Bloque TEST 3:** Llama a tu función `isValid`. Si tu función `fix` y tu `isValid` tienen lógicas matemáticas contradictorias, este test fallará.

**Bloque TEST 4:** Comprueba que la covarianza y los logaritmos no produzcan errores matemáticos como divisiones por cero o raíces negativas (que en C++ se representan como `NaN`, Not a Number).


---
ERROR QUE ME DABA 
---

¡Enhorabuena! Ver la consola completamente en verde tras una sesión intensa de depuración es una de las mayores satisfacciones en la carrera de un ingeniero informático. 

Es completamente normal que la conjunción de números aleatorios y límites estrictos te haya resultado confusa. En Metaheurísticas, a menudo luchamos contra la naturaleza estocástica (aleatoria) de nuestros propios algoritmos. Como tu profesor, he preparado este informe técnico detallado para que comprendas "con pelos y señales" qué estaba fallando y por qué nuestra solución lo ha arreglado definitivamente.

---

## Informe Técnico: Depuración del Modelo de Portfolio

El problema subyacente que causaba el fallo intermitente (unas veces en la iteración 4, otras en la 12) se dividía en dos fenómenos matemáticos y computacionales muy concretos.

### 1. La Paradoja de los Ceros (Falta de Liquidez Legal)

El guion establece unas reglas de oro muy estrictas para considerar una solución como válida:
* [cite_start]La suma de los pesos debe ser $1$[cite: 42, 43].
* [cite_start]Cada peso debe ser $0$ o estar dentro del rango $lo \le w_{i} \le hi$[cite: 46].

**El escenario del colapso:**
[cite_start]Piensa en el IBEX 35. Tenemos 30 empresas, con un límite de inversión máximo por empresa de $hi = 0.08$ (un 8%)[cite: 73, 74]. [cite_start]Tu algoritmo de generación aleatoria escoge un número de ceros al azar[cite: 83]. 
Imagina que, por puro azar, el generador decide poner 27 ceros. Esto significa que solo vas a invertir en 3 empresas. Incluso si inviertes el máximo legal en esas 3 empresas ($3 \cdot 0.08$), tu inversión total sumará **0.24**. Te falta un **0.76** para llegar a la suma obligatoria de $1$.

**El error del algoritmo antiguo:**
Al ver que faltaba dinero, el antiguo algoritmo buscaba a quién darle ese 0.76. Como las 3 empresas activas ya estaban al límite ($hi$), decidía repartir el dinero entre los ceros. Dividía $0.76 / 27 \approx 0.028$. Hasta aquí, todo bien.
Pero, ¿qué pasaba si la diferencia a repartir era ínfima? Imagina que faltaban solo **0.05** por repartir entre 27 ceros. El algoritmo les daba a cada uno $0.05 / 27 \approx 0.0018$. 
[cite_start]Al pasar por el auditor `isValid`, este detectaba que $0.0018$ era mayor que 0 pero estrictamente menor que el límite inferior $lo = 0.005$ del IBEX 35[cite: 74]. ¡Habíamos creado inversiones fantasma ilegales! Y el test fallaba y abortaba el programa.

### 2. La Maldición de la Coma Flotante (IEEE 754)

Los ordenadores no pueden representar números decimales infinitos de forma exacta en memoria. Al sumar fracciones repetidamente, se acumula "basura decimal".
En tu código, cuando una empresa se topaba con su límite máximo ($0.08$), al hacer las sumas y restas de la reparación, el ordenador acababa almacenando internamente algo como **0.0800000000000001**.
Tu antigua función `isValid` preguntaba fríamente: *¿Es este peso mayor que $hi$?* Matemáticamente sí. Por un margen microscópico, pero lo es. Y por tanto, te marcaba una solución excelente como inválida.

---

## La Ingeniería detrás de la Solución

Para arreglar esto, tuvimos que aplicar dos conceptos avanzados de programación numérica:

* **Tolerancia Computacional ($\epsilon$):** En la nueva función `isValid`, en lugar de preguntar si $w_{i} > hi$, le dimos un "colchón" de tolerancia de $10^{-8}$. Ahora preguntamos si $w_{i} > hi + 10^{-8}$. Esto ignora la basura decimal y acepta los topes correctos. [cite_start]El guion de hecho ya sugería usar este margen de error para la suma[cite: 43, 45].
* **Resurrección Atómica Controlada:** En la función `fix`, cambiamos radicalmente cómo resucitamos a los ceros cuando nos falta dinero y nadie más puede absorberlo. En lugar de darles "lo que sobre", ahora elegimos **un solo cero al azar** y le asignamos exactamente la inversión mínima legal ($w_{i} = lo$). Forzamos un recalculo (`continue`) y, si en la siguiente iteración aún sobra dinero, ese activo ya estará legalmente "vivo" para absorber más capital de forma proporcional. Esto garantiza matemáticamente que jamás generaremos un peso que viole la restricción mínima.

---
