#!/usr/bin/php-cgi
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>PHP Hello, World</title>
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-sRIl4kxILFvY47J16cr9ZwB07vP4J8+LH7qKQnuqkuIAvNWLzeN8tE5YBujZqJLB" crossorigin="anonymous">
        <style>
        div {
            background-size: cover;
            background-position: center;
            height: 100vh;
            <?php if ($_SERVER["REQUEST_METHOD"] === "GET") { ?>
             background-image: url('https://media.tenor.com/KshIPrRS1aAAAAAM/cat-orange-cat.gif');
            <?php } else { ?>
             background-image: url('https://c.tenor.com/r0D9HVUsnxsAAAAd/tenor.gif');
            <?php } ?>
        }

        h1 {
        font-size: 6rem !important;
            line-height: 1.8 !important;
        }
        section label {
            font-size: 3rem !important;
        }
        section input {
            font-size: 3rem !important;
        }
        </style>
    </head>
    <body>
        <div class="container-fluid text-center">
            <div class="row justify-content-md-center align-items-center">
                <main class="col text-white fw-bold">
                    <?php if ($_SERVER["REQUEST_METHOD"] === "POST") {
                        echo "<h1>Hello, " . htmlspecialchars($_POST["name"]) . "!</h1>";
                    } else { ?>

                    <form action="<?php $_SERVER["PATH_TRANSLATED"]; ?>" method="post">
                        <section>
                            <label for="name">Name: </label>
                            <input type="text" id="name" class="text-dark" name="name" value="" placeholder="<name>"/>
                            <input type="submit" id="submit" value="Send" />
                        </section>
                    </form>
                    <?php } ?>
                </main>
            </div>
        </div>
    </body>
</html>
