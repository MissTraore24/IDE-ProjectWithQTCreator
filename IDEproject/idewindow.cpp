#include "idewindow.h"
#include "ui_idewindow.h"
#include <QFileDialog>
#include <QFile>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QTextStream>
#include <QCompleter>
#include "highlighter.h"
#include <QStringListModel>
#include <QKeyEvent>
#include<QFileIconProvider>
#include <QTabWidget>
#include <QMessageBox>
#include <QDockWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QToolBar>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QToolTip>
#include <QProcess>
#include <QDir>
#include <QFileSystemWatcher>

IDEWindow::IDEWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::IDEWindow) {
    ui->setupUi(this);

    // Créer un QTabWidget pour gérer les onglets
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    // Créer la console et la placer dans un QDockWidget
    ui->console = new QPlainTextEdit(this);
    ui->console->setReadOnly(true);

    QDockWidget *consoleDockWidget = new QDockWidget("Console", this);
    consoleDockWidget->setWidget(ui->console);
    consoleDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
    consoleDockWidget->setMinimumHeight(150);
    addDockWidget(Qt::BottomDockWidgetArea, consoleDockWidget);


    QAction *toggleConsoleAction = new QAction("Afficher/Masquer Console", this);
    toggleConsoleAction->setCheckable(true);
    toggleConsoleAction->setChecked(true);  // Initialement, la console est visible
    connect(toggleConsoleAction, &QAction::toggled, this, &IDEWindow::toggleConsole);
    ui->menuAffichage->addAction(toggleConsoleAction);



    // Créer l'explorateur de projet
    projectExplorer = new QTreeView(this);
    fileModel = new QFileSystemModel(this);
    fileModel->setRootPath("");  // Démarrer avec un explorateur vide
    projectExplorer->setModel(fileModel);
    projectExplorer->setHeaderHidden(true);

    // Afficher des icônes pour les fichiers et dossiers
    fileModel->setIconProvider(new QFileIconProvider());
    projectExplorer->setModel(fileModel);

    // Connecter la sélection de fichier dans l'explorateur avec l'ouverture d'onglets
    connect(projectExplorer, &QTreeView::doubleClicked, this, [&](const QModelIndex &index) {
        QString filePath = fileModel->filePath(index);
        if (QFileInfo(filePath).isFile()) {
            openFile(filePath);  // Ouvrir le fichier si c'est un fichier
        } else {
            projectExplorer->setRootIndex(fileModel->index(filePath));  // Naviguer dans les dossiers
        }
    });

    // Créer un DockWidget pour l'explorateur de projet
    QDockWidget *explorerDockWidget = new QDockWidget("Explorateur", this);
    explorerDockWidget->setWidget(projectExplorer);
    explorerDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    explorerDockWidget->setMinimumWidth(250);  // Optionnel: définir la largeur minimale
    addDockWidget(Qt::LeftDockWidgetArea, explorerDockWidget);

    // Menu contextuel (clic droit)
    projectExplorer->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(projectExplorer, &QTreeView::customContextMenuRequested, this, &IDEWindow::showContextMenu);

    // Drag-and-drop
    projectExplorer->setDragEnabled(true);
    projectExplorer->setDropIndicatorShown(true);
    projectExplorer->setAcceptDrops(true);

    // Auto-refresh avec QFileSystemWatcher
    QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &IDEWindow::refreshExplorer);
    watcher->addPath(QDir::homePath());  // Remplacez par le chemin que vous souhaitez surveiller

    // Connecter les actions du menu
    connect(ui->actionOuvrir, &QAction::triggered, this, &IDEWindow::openFileDialog);
    connect(ui->actionEnregistrer, &QAction::triggered, this, &IDEWindow::saveFile);
    connect(ui->actionExecuter, &QAction::triggered, this, &IDEWindow::runCode);
    connect(ui->actionQuitter, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionImporter_Dossier, &QAction::triggered, this, &IDEWindow::importFolder);

    connect(ui->actionCopier, &QAction::triggered, this, &IDEWindow::copierTexte);
    connect(ui->actionColler, &QAction::triggered, this, &IDEWindow::collerTexte);
    connect(ui->actionCouper, &QAction::triggered, this, &IDEWindow::couperTexte);


    setupCompleter();

    // Gérer la fermeture d'onglets
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &IDEWindow::closeTab);
}

IDEWindow::~IDEWindow() {
    delete ui;
    delete tabWidget;
    delete completer;
}

void IDEWindow::showFilePathTooltip(const QModelIndex &index) {
    QString filePath = fileModel->filePath(index);
    QToolTip::showText(QCursor::pos(), filePath);  // Afficher l'infobulle avec le chemin complet
}
void IDEWindow::toggleConsole(bool checked) {
    if (checked) {
        ui->console->show();  // Afficher la console
    } else {
        ui->console->hide();  // Masquer la console
    }
}

void IDEWindow::importFolder() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Importer un dossier", "");
    if (!dirPath.isEmpty()) {
        projectExplorer->setRootIndex(fileModel->index(dirPath));  // Afficher le dossier importé
    }
}

void IDEWindow::openFileDialog() {
    QString fileName = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", "", "Fichiers source (*.cpp *.py *.js)");
    if (!fileName.isEmpty()) {
        openFile(fileName);
    }
}

void IDEWindow::openFile(const QString &fileName) {
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QPlainTextEdit *editor = new QPlainTextEdit;
            editor->setPlainText(in.readAll());

            // Ajouter le fichier dans un nouvel onglet
            tabWidget->addTab(editor, QFileInfo(fileName).fileName());

            file.close();
            currentFilePath = fileName;

            // Ouvrir le fichier et l'ajouter à l'explorateur
            projectExplorer->setRootIndex(fileModel->index(QFileInfo(fileName).absolutePath()));

            // Appliquer des règles de surlignage selon le type de fichier
            if (fileName.endsWith(".cpp")) {
                Highlighter *cppHighlighter = new Highlighter(editor->document());
            } else if (fileName.endsWith(".py")) {
                Highlighter *pythonHighlighter = new Highlighter(editor->document());
            } else if (fileName.endsWith(".js")) {
                Highlighter *jsHighlighter = new Highlighter(editor->document());
            }
        }
    }
}

void IDEWindow::saveFile() {
    if (tabWidget->count() == 0) {
        QMessageBox::warning(this, "Avertissement", "Aucun fichier ouvert.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Enregistrer sous", "", "Fichiers source (*.cpp *.py *.js)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(tabWidget->currentWidget());
            if (editor) {
                out << editor->toPlainText();
            }
            file.close();
            currentFilePath = fileName;
        }
    }
}

void IDEWindow::runCode() {
    if (currentFilePath.isEmpty()) {
        ui->console->appendPlainText("Veuillez enregistrer le fichier avant de l'exécuter.");
        return;
    }

    process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, this, [&]() {
        ui->console->appendPlainText(process->readAllStandardOutput());
    });

    connect(process, &QProcess::readyReadStandardError, this, [&]() {
        ui->console->appendPlainText(process->readAllStandardError());
    });

    if (currentFilePath.endsWith(".cpp")) {
        QStringList compileArgs;
        compileArgs << currentFilePath << "-o" << "output";
        process->start("g++", compileArgs);

        process->waitForFinished();
        if (process->exitCode() == 0) {
            process->start("./output");
        }
    } else if (currentFilePath.endsWith(".py")) {
        process->start("python", QStringList() << currentFilePath);
    } else if (currentFilePath.endsWith(".js")) {
        process->start("node", QStringList() << currentFilePath);
    }
}

void IDEWindow::setupCompleter() {
    keywords << "int" << "float" << "double" << "return" << "for" << "while" << "if" << "else";

    completer = new QCompleter(keywords, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
}

void IDEWindow::closeTab(int index) {
    if (index >= 0) {
        tabWidget->removeTab(index);
    }
}

void IDEWindow::refreshExplorer() {
    projectExplorer->setRootIndex(fileModel->index(fileModel->rootPath()));  // Mettre à jour l'explorateur
}

void IDEWindow::showContextMenu(const QPoint &pos) {
    QMenu contextMenu(tr("Menu contextuel"), this);
    QAction *openAction = new QAction(tr("Ouvrir"), this);
    connect(openAction, &QAction::triggered, this, [&]() {
        QModelIndex index = projectExplorer->currentIndex();
        if (index.isValid()) {
            openFile(fileModel->filePath(index));
        }
    });
    contextMenu.addAction(openAction);

    QAction *deleteAction = new QAction(tr("Supprimer"), this);
    connect(deleteAction, &QAction::triggered, this, [&]() {
        QModelIndex index = projectExplorer->currentIndex();
        if (index.isValid()) {
            QString filePath = fileModel->filePath(index);
            QFile::remove(filePath);
            refreshExplorer();  // Rafraîchir l'explorateur
        }
    });
    contextMenu.addAction(deleteAction);

    contextMenu.exec(projectExplorer->viewport()->mapToGlobal(pos));  // Exécuter le menu contextuel
}
// Ajout des actions Copier, Couper, Coller
void IDEWindow::copierTexte() {
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit *>(tabWidget->currentWidget());
    if (editor) {
        editor->copy();  // Utilise la méthode intégrée pour copier le texte sélectionné
    } else {
        QMessageBox::warning(this, "Erreur", "Aucun éditeur actif pour copier du texte.");
    }
}

void IDEWindow::collerTexte() {
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit *>(tabWidget->currentWidget());
    if (editor) {
        editor->paste();  // Utilise la méthode intégrée pour coller le texte depuis le presse-papiers
    } else {
        QMessageBox::warning(this, "Erreur", "Aucun éditeur actif pour coller du texte.");
    }
}

void IDEWindow::couperTexte() {
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit *>(tabWidget->currentWidget());
    if (editor) {
        editor->cut();  // Utilise la méthode intégrée pour couper le texte sélectionné
    } else {
        QMessageBox::warning(this, "Erreur", "Aucun éditeur actif pour couper du texte.");
    }
}

