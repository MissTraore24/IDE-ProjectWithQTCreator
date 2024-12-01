#ifndef IDEWINDOW_H
#define IDEWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QProcess>
#include <QPushButton>
#include <QCompleter>
#include <QTreeView>
#include <QFileSystemModel>
#include <QFileDialog>

namespace Ui {
class IDEWindow;
}

class IDEWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit IDEWindow(QWidget *parent = nullptr);
    ~IDEWindow();

private slots:
    void openFile(const QString &fileName);    // Ouvrir un fichier
    void saveFile();                           // Enregistrer un fichier
    void openFileDialog();                     // Boîte de dialogue pour ouvrir un fichier
    void runCode();                            // Exécuter le code
    void closeTab(int index);                  // Fermer l'onglet sélectionné
    void showContextMenu(const QPoint &pos);   // Afficher le menu contextuel dans l'explorateur
    void importFile();                         // Importer un fichier
    void importFolder();                       // Importer un dossier
    void openProjectFolder();                  // Ouvrir un dossier de projet spécifique
    void updateProjectExplorer(const QString &path); // Mettre à jour l'explorateur avec le dossier du projet
    void createFolder();
    void createFile();
    void showFilePathTooltip(const QModelIndex &index); // Méthode pour infobulle
    void refreshExplorer();
    void toggleConsole(bool checked);
     void setupEditActions();
     void copierTexte();
        void collerTexte();
        void couperTexte();


private:
    Ui::IDEWindow *ui;
    QString currentFilePath;                   // Chemin du fichier actuellement ouvert
    QProcess *process;                         // Processus pour exécuter des commandes
    QTabWidget *tabWidget;                     // Widget pour gérer les onglets
    QPlainTextEdit *console;                   // Console pour afficher la sortie
    QTreeView *projectExplorer;                // Explorateur de projet
    QFileSystemModel *fileModel;               // Modèle pour l'explorateur de fichiers
    QString projectDirectory;                  // Répertoire de projet actuel
    QStringList keywords;                      // Liste des mots-clés pour l'autocomplétion
    QCompleter *completer;                      // Gestionnaire d'autocomplétion

    void setupCompleter();                     // Initialiser l'autocomplétion
};

#endif // IDEWINDOW_H
