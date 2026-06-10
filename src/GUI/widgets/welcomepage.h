#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QStringList>

class WelcomePage : public QWidget {
	Q_OBJECT
public:
	explicit WelcomePage(QWidget* parent = nullptr);

	void setRecentFiles(const QStringList& files);
	void addRecentFile(const QString& path);
	QStringList recentFiles() const;

signals:
	void fileSelected(const QString& path);
	void openFileClicked();

protected:
	void showEvent(QShowEvent* event) override;

private slots:
	void onRecentDoubleClicked(QListWidgetItem* item);
	void onQuickOpenReturn();

private:
	void setupUI();
	void loadRecentFiles();
	void saveRecentFiles();
	void animateEntrance();

	QLabel* titleLabel;
	QLineEdit* quickOpen;
	QListWidget* recentList;
	QPushButton* openBtn;
	QStringList recentPaths;
	static const int MAX_RECENT = 10;
};

#endif // WELCOMEPAGE_H
